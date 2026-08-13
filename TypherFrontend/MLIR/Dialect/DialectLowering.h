using namespace MLIR;

struct StoreOpLowering : public mlir::OpConversionPattern<mlir::typher::StoreOp> {
    using mlir::OpConversionPattern<mlir::typher::StoreOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(mlir::typher::StoreOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        
        // Adaptor operands are already converted to LLVM-compatible types
        mlir::Value valueToStore = adaptor.getValueToStore();
        mlir::Value addr = adaptor.getAddr();

        // Create llvm.store %val, %ptr
        rewriter.create<mlir::LLVM::StoreOp>(
            op.getLoc(), 
            valueToStore, 
            addr
        );

        // Erase the original typher.store (store produces no results)
        rewriter.eraseOp(op);
        return mlir::success();
    }
};


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
        auto loc = op->getLoc();

        mlir::Type origAllocatedType;
        mlir::Type addrType = op.getAddr().getType();

        if (auto ptrType = mlir::dyn_cast<mlir::typher::PointerType>(addrType)) {
            origAllocatedType = ptrType.getElementType(); // e.g. !typher.array<3 x i32> or i32
        } else if (auto memrefType = mlir::dyn_cast<mlir::MemRefType>(addrType)) {
            origAllocatedType = memrefType.getElementType();
        } else {
            return rewriter.notifyMatchFailure(op, "unsupported address type");
        }

        mlir::Type llvmAllocatedType = typeConverter->convertType(origAllocatedType);
        if (!llvmAllocatedType) {
            return rewriter.notifyMatchFailure(op, "failed to convert allocated type to LLVM");
        }

        auto one = rewriter.create<mlir::LLVM::ConstantOp>(
            loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));

        auto resultType = typeConverter->convertType(addrType); // !llvm.ptr
        auto llvmPtr = rewriter.create<mlir::LLVM::AllocaOp>(
            loc, resultType, llvmAllocatedType, one);

        rewriter.replaceOp(op, llvmPtr.getResult());
        return mlir::success();
    }
};

struct LoadLowering : public OpConversionPattern<typher::LoadOp> {
    using OpConversionPattern<typher::LoadOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::LoadOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        // 1. Get the converted !llvm.ptr address
        mlir::Value llvmPtr = adaptor.getAddr();

        // 2. Convert result type to LLVM type (e.g. i32)
        mlir::Type resType = getTypeConverter()->convertType(op.getValue().getType());
        if (!resType)
            return mlir::failure();

        // 3. Lower typher.load -> llvm.load
        rewriter.replaceOpWithNewOp<mlir::LLVM::LoadOp>(op, resType, llvmPtr);
        
        return mlir::success();
    }
};

struct AssignLowering : public OpConversionPattern<typher::AssignOp> {
    using OpConversionPattern<typher::AssignOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::AssignOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        auto loc = op.getLoc();
        mlir::Value targetAddr = adaptor.getAddr();
        mlir::Value valueToStore = adaptor.getValue();

        // Check the original type of the expression before lowering to !llvm.ptr
        mlir::Type origType = op.getValue().getType();

        if (auto ptrType = mlir::dyn_cast<typher::PointerType>(origType)) {
            origType = ptrType.getElementType();
        }

        if (auto arrayType = mlir::dyn_cast<typher::ArrayType>(origType)) {
            // -----------------------------------------------------------------
            // AGGREGATE COPY: Emit llvm.intr.memcpy
            // -----------------------------------------------------------------
            int64_t numElements = arrayType.getSize();
            
            mlir::Type convertedElemType = getTypeConverter()->convertType(arrayType.getElementType());
            
            int64_t elemSize = 4; // Default to 4 bytes for i32
            if (convertedElemType.isInteger(64) || convertedElemType.isF64()) {
                elemSize = 8;
            } else if (convertedElemType.isInteger(8)) {
                elemSize = 1;
            }

            int64_t totalBytes = numElements * elemSize;

            mlir::Value bytesVal = rewriter.create<mlir::LLVM::ConstantOp>(
                loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(totalBytes)
            );

            // ✅ Fix: Pass bool literal for isVolatile
            rewriter.create<mlir::LLVM::MemcpyOp>(
                loc,
                targetAddr,   // dst
                valueToStore, // src
                bytesVal,     // len
                /*isVolatile=*/false
            );
        } else {
            rewriter.create<mlir::LLVM::StoreOp>(loc, valueToStore, targetAddr);
        }

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

        if (mlir::isa<mlir::typher::PointerType>(lhs.getType())) {
            lhs = rewriter.create<mlir::memref::LoadOp>(loc, lhs);
        }

        if (mlir::isa<mlir::typher::PointerType>(rhs.getType())) {
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

    for (auto [idx, argType] : llvm::enumerate(op.getArgumentTypes())) {
        auto llvmType = getTypeConverter()->convertType(argType);
        signatureConversion.addInputs(idx, llvmType);
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
        if (mlir::isa<mlir::typher::PointerType>(returnValue.getType())) {
            // Dereference the memref to get the actual i32 value
            auto i32Type = rewriter.getI32Type();
            mlir::Value loadedVal = rewriter.create<mlir::LLVM::LoadOp>(loc, i32Type, returnValue);
        }

        rewriter.replaceOpWithNewOp<mlir::func::ReturnOp>(op, returnValue);
        return success();
    }
};


struct AccessOpLowering : public mlir::OpConversionPattern<mlir::typher::AccessOp> {
    using OpConversionPattern<mlir::typher::AccessOp>::OpConversionPattern;

    mlir::LogicalResult matchAndRewrite(
        mlir::typher::AccessOp op,
        OpAdaptor adaptor,
        mlir::ConversionPatternRewriter &rewriter) const override 
    {
        mlir::Value llvmBasePtr = adaptor.getBase();
        
        // adaptor.getIndices() contains the values AFTER type conversion (already converted to i64!)
        auto llvmIndices = adaptor.getIndices(); 

        auto typherPtrType = llvm::cast<mlir::typher::PointerType>(op.getBase().getType());
        mlir::Type llvmElementType = getTypeConverter()->convertType(typherPtrType.getElementType());

        if (!llvmElementType) {
            return rewriter.notifyMatchFailure(op, "Failed to convert element type");
        }

        rewriter.replaceOpWithNewOp<mlir::LLVM::GEPOp>(
            op,
            mlir::LLVM::LLVMPointerType::get(getContext()), // Result: !llvm.ptr
            llvmElementType,                                 // Pointee element type
            llvmBasePtr,                                     // Converted base
            llvmIndices                                      // Converted indices (i64)
        );

        return mlir::success();
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
