using namespace MLIR;

template <typename BinaryOp, typename LoweredBinaryOp>
struct BinaryOpLowering : public OpConversionPattern<BinaryOp> {
    using OpConversionPattern<BinaryOp>::OpConversionPattern;
    using OpAdaptor = typename OpConversionPattern<BinaryOp>::OpAdaptor;

    LogicalResult matchAndRewrite(BinaryOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const {
        auto loc = op->getLoc();
        Value lhs = adaptor.getLhs();
        Value rhs = adaptor.getRhs();

        if (!llvm::isa<IntegerType>(lhs.getType())) {
            return rewriter.notifyMatchFailure(op, "Expected scalar integer types.");
        }

        auto scalarResult = rewriter.create<arith::AddIOp>(loc, lhs, rhs);

        rewriter.replaceOp(op, scalarResult.getResult());
        return success();
    }
};

struct ConstantOpLowering : public OpConversionPattern<mlir::typher::ConstantOp> {
    using OpConversionPattern<mlir::typher::ConstantOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::typher::ConstantOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        Attribute valueAttr = op.getValueAttr();
        if (!llvm::isa<IntegerAttr>(valueAttr) && !llvm::isa<FloatAttr>(valueAttr)) {
            return rewriter.notifyMatchFailure(op, "Expected integer attribute.");
        }

        rewriter.replaceOpWithNewOp<arith::ConstantOp>(
            op, 
            op.getType(), 
            llvm::cast<TypedAttr>(valueAttr)
        );
        return success();
    }
};

struct AllocaLowering : public OpConversionPattern<typher::AllocaOp> {
    using OpConversionPattern<typher::AllocaOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::AllocaOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        auto allocaOp = mlir::cast<typher::AllocaOp>(op);
        auto loc = op->getLoc();

        // 1. Get the underlying element type (e.g., i32 from memref<i32>)
        auto memrefType = mlir::cast<mlir::MemRefType>(allocaOp.getAddr().getType());
        auto elementType = typeConverter->convertType(memrefType.getElementType());;

        // 2. Create a constant '1' for the number of elements to allocate
        auto one = rewriter.create<mlir::LLVM::ConstantOp>(
            loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));

        // 3. Create the LLVM Alloca (returns a !llvm.ptr)
        auto llvmPtr = rewriter.create<mlir::LLVM::AllocaOp>(
            loc, mlir::LLVM::LLVMPointerType::get(getContext()), elementType, one);

        rewriter.replaceOp(op, llvmPtr);
        return mlir::success();
    }
};

struct LoadLowering : public OpConversionPattern<typher::LoadOp> {
    using OpConversionPattern<typher::LoadOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::LoadOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        
        auto loc = op.getLoc();

        // 1. Get the already-converted !llvm.ptr from the adaptor
        // (This was produced by your AllocaLowering)
        mlir::Value llvmPtr = adaptor.getAddr();

        // 2. Determine the result type (e.g., i32)
        // We ask the typeConverter to make sure i32 is valid for LLVM
        mlir::Type resType = typeConverter->convertType(op.getValue().getType());

        // 3. Replace typher.load with llvm.load
        // Modern LLVM Load requires: (location, resultType, pointer)
        rewriter.replaceOpWithNewOp<mlir::LLVM::LoadOp>(op, resType, llvmPtr);
        
        return mlir::success();
    }
};

struct AssignLowering : public OpConversionPattern<typher::AssignOp> {
    using OpConversionPattern<typher::AssignOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::AssignOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        mlir::Value targetAddr = adaptor.getAddr();
        mlir::Value valueToStore = adaptor.getValue();
        // Perform the LLVM store
        rewriter.create<mlir::LLVM::StoreOp>(op.getLoc(), valueToStore, targetAddr);

        rewriter.eraseOp(op);
        return mlir::success();
    }
};

struct IfOpLowering : public OpConversionPattern<typher::IfOp> {
    using OpConversionPattern<typher::IfOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(typher::IfOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override
    {
        auto loc = op.getLoc();

        auto fixTerminator = [&](Block *block, Block *dest) {
            if (block->empty()) return;

            Operation *terminator = block->getTerminator();

            if (auto yieldOp = llvm::dyn_cast<typher::YieldOp>(terminator)) {
                rewriter.setInsertionPoint(yieldOp);
                rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(yieldOp, dest);
            }
        };

        Block *currentBlock = rewriter.getInsertionBlock();
        Block *continuationBlock = rewriter.splitBlock(currentBlock, op->getIterator());
        Block *thenBlock = &op.getThenRegion().front();
        Block *elseBlock = op.getElseRegion().empty() 
                            ? continuationBlock 
                            : &op.getElseRegion().front();

        rewriter.setInsertionPointToEnd(currentBlock);
        rewriter.create<mlir::cf::CondBranchOp>(loc, adaptor.getCondition(), thenBlock, elseBlock);

        rewriter.inlineRegionBefore(op.getThenRegion(), continuationBlock);

        fixTerminator(thenBlock, continuationBlock);
        if (!op.getElseRegion().empty()) {
            fixTerminator(elseBlock, continuationBlock);
            rewriter.inlineRegionBefore(op.getElseRegion(), continuationBlock);
        }

        rewriter.eraseOp(op);
        return success();
    }
};

struct WhileLowering : public OpConversionPattern<typher::WhileOp> {
    using OpConversionPattern<typher::WhileOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(typher::WhileOp op, OpAdaptor adaptor,
                                 ConversionPatternRewriter &rewriter) const override {
        auto loc = op.getLoc();

        // 1. Safety Check: Ensure regions aren't empty
        if (op.getCondRegion().empty() || op.getBodyRegion().empty())
            return failure();

        // 2. Identify the surrounding context
        Block *currentBlock = rewriter.getInsertionBlock();
        
        // Split block at the 'op' itself to create the exit point
        Block *continuationBlock = rewriter.splitBlock(currentBlock, Block::iterator(op));

        // 3. Prepare the new Blocks
        // We move the existing blocks out of the regions into the Function
        Block *condBlock = &op.getCondRegion().front();
        Block *bodyBlock = &op.getBodyRegion().front();

        // Inline them before the continuation point
        rewriter.inlineRegionBefore(op.getCondRegion(), continuationBlock);
        rewriter.inlineRegionBefore(op.getBodyRegion(), continuationBlock);

        // 4. Wire the Entry -> Condition
        rewriter.setInsertionPointToEnd(currentBlock);
        rewriter.create<mlir::cf::BranchOp>(loc, condBlock);

        // 5. Wire the Condition -> Body OR Exit
        Operation *condTerminator = condBlock->getTerminator();
        // Use the yield value as the branch condition
        Value condition = condTerminator->getOperand(0); 

        rewriter.setInsertionPointToEnd(condBlock);
        rewriter.replaceOpWithNewOp<mlir::cf::CondBranchOp>(
            condTerminator, condition, bodyBlock, continuationBlock);

        // 6. Wire the Body -> Condition (The Back-edge)
        Operation *bodyTerminator = bodyBlock->getTerminator();
        rewriter.setInsertionPointToEnd(bodyBlock);
        rewriter.replaceOpWithNewOp<mlir::cf::BranchOp>(bodyTerminator, condBlock);

        // 7. Success
        rewriter.eraseOp(op);
        return success();
    }
};

struct EqualsOpLowering : public mlir::OpConversionPattern<typher::EqualsOp> {
    using OpConversionPattern<typher::EqualsOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::EqualsOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        auto loc = op.getLoc();
        mlir::Value lhs = adaptor.getLhs();
        mlir::Value rhs = adaptor.getRhs();

        // 1. Check if they are memrefs. If so, load the value.
        // Assuming these are 0-dimensional memrefs (like a pointer to a single int)
        if (mlir::isa<mlir::MemRefType>(lhs.getType())) {
            // Note: If you're using 'mlir' namespace, you can just use 'isa'
            lhs = rewriter.create<mlir::memref::LoadOp>(loc, lhs);
        }
        if (mlir::isa<mlir::MemRefType>(rhs.getType())) {
            // Note: If you're using 'mlir' namespace, you can just use 'isa'
            rhs = rewriter.create<mlir::memref::LoadOp>(loc, rhs);
        }

        // 2. Now both lhs and rhs are 'i32' (signless-integer-like)
        rewriter.replaceOpWithNewOp<mlir::arith::CmpIOp>(
            op, 
            mlir::arith::CmpIPredicate::eq, 
            lhs, 
            rhs
        );

        return mlir::success();
    }
};

struct FuncOpLowering : public OpConversionPattern<mlir::typher::FuncOp> {
  using OpConversionPattern<mlir::typher::FuncOp>::OpConversionPattern;

  LogicalResult matchAndRewrite(mlir::typher::FuncOp op, OpAdaptor adaptor,
                                ConversionPatternRewriter &rewriter) const override {
    const TypeConverter *converter = getTypeConverter();
    if (!converter) {
      return rewriter.notifyMatchFailure(op, "Type converter missing in pattern");
    }

    TypeConverter::SignatureConversion signatureConversion(op.getNumArguments());
    SmallVector<Type, 4> convertedResults;

    for (auto argType : op.getFunctionType().getInputs()) {
        SmallVector<Type, 1> convertedTypes;
        if (failed(converter->convertType(argType, convertedTypes))) {
            return rewriter.notifyMatchFailure(op, "Failed to convert input argument type");
        }
        // This registers the mapping so getConvertedTypes() actually has data!
        signatureConversion.addInputs(convertedTypes);
    }
    
    if (failed(converter->convertTypes(op.getFunctionType().getResults(), convertedResults))) {
      return rewriter.notifyMatchFailure(op, "Failed to convert function signature types");
    }

    auto newFuncType = rewriter.getFunctionType(
        signatureConversion.getConvertedTypes(), 
        convertedResults
    );

    auto func = rewriter.create<mlir::func::FuncOp>(
        op.getLoc(), 
        op.getName(), 
        newFuncType
    );

    rewriter.inlineRegionBefore(op.getRegion(), func.getBody(), func.end());

    if (failed(rewriter.convertRegionTypes(&func.getBody(), *converter, &signatureConversion))) {
      return rewriter.notifyMatchFailure(op, "Failed to rewrite region block signatures");
    }

    rewriter.eraseOp(op);
    return success();
  }
};


struct ReturnOpLowering : public OpConversionPattern<mlir::typher::ReturnOp> {
    using OpConversionPattern<mlir::typher::ReturnOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::typher::ReturnOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const
    {
        auto loc = op.getLoc();
        mlir::Value returnValue = adaptor.getOperands()[0];

        // If we are trying to return a memref to a function expecting an i32:
        if (mlir::isa<mlir::MemRefType>(returnValue.getType())) {
            // Dereference the memref to get the actual i32 value
            auto i32Type = rewriter.getI32Type();
            mlir::Value loadedVal = rewriter.create<mlir::LLVM::LoadOp>(loc, i32Type, returnValue);
        }

        rewriter.replaceOpWithNewOp<mlir::func::ReturnOp>(op, returnValue);
        return success();
    }
};


struct CallOpLowering : public OpConversionPattern<mlir::typher::GenericCallOp> {
    using OpConversionPattern<mlir::typher::GenericCallOp>::OpConversionPattern;

    LogicalResult
    matchAndRewrite(mlir::typher::GenericCallOp op, OpAdaptor adaptor,
                    ConversionPatternRewriter &rewriter) const override
    {
        // Use adaptor.getOperands() - these are the already lowered scalar integers
        auto callOp = rewriter.create<func::CallOp>(
            op.getLoc(),
            op.getCallee(),      // The SymbolRef (function name)
            op.getResultTypes(),    // The new standard result types
            adaptor.getOperands() // The new standard argument values
        );

        // 3. Replace the old op with the new call's results
        rewriter.replaceOp(op, callOp.getResults());
        return success();
    }
};
