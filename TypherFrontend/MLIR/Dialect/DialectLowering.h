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

using AddOpLowering = BinaryOpLowering<mlir::typher::AddOp, arith::AddFOp>;
// using MulOpLowering = BinaryOpLowering<mlir::typher::::MulOp, arith::MulFOp>;

struct ConstantOpLowering : public OpConversionPattern<mlir::typher::ConstantOp> {
    using OpConversionPattern<mlir::typher::ConstantOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::typher::ConstantOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
        Attribute valueAttr = op.getValueAttr();
        if (!llvm::isa<IntegerAttr>(valueAttr)) {
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

struct AllocaLowering : public OpRewritePattern<typher::AllocaOp> {
    using OpRewritePattern<typher::AllocaOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(typher::AllocaOp op, 
                                  PatternRewriter &rewriter) const override {
        // Just replace typher.alloca with memref.alloca
        // The result type (memref<i32>) remains the same.
        rewriter.replaceOpWithNewOp<memref::AllocaOp>(
            op, op.getType());
        return success();
    }
};

struct AssignLowering : public OpRewritePattern<typher::AssignOp> {
    using OpRewritePattern<typher::AssignOp>::OpRewritePattern;

    LogicalResult matchAndRewrite(typher::AssignOp op, 
                                  PatternRewriter &rewriter) const override {
        // 1. Create the store: memref.store %value, %addr
        rewriter.create<memref::StoreOp>(
            op.getLoc(), op.getValue(), op.getAddr());

        // 2. If your AssignOp returns a value (like C does), 
        // replace usage of the result with the input value itself.
        if (op->getNumResults() > 0) {
            rewriter.replaceOp(op, op.getValue());
        } else {
            rewriter.eraseOp(op);
        }
        return success();
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

struct EqualsOpLowering : public mlir::OpConversionPattern<typher::EqualsOp> {
    using OpConversionPattern<typher::EqualsOp>::OpConversionPattern;

    mlir::LogicalResult
    matchAndRewrite(typher::EqualsOp op, OpAdaptor adaptor,
                    mlir::ConversionPatternRewriter &rewriter) const override {
        
        // We replace typher.eq with arith.cmpi
        // adaptor.getLhs() and adaptor.getRhs() give us the 
        // already-lowered operands.
        rewriter.replaceOpWithNewOp<mlir::arith::CmpIOp>(
            op, 
            mlir::arith::CmpIPredicate::eq, // This makes it an "equals" check
            adaptor.getLhs(), 
            adaptor.getRhs()
        );

        return mlir::success();
    }
};

struct FuncOpLowering : public OpConversionPattern<mlir::typher::FuncOp> {
  using OpConversionPattern<mlir::typher::FuncOp>::OpConversionPattern;

  LogicalResult matchAndRewrite(mlir::typher::FuncOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const {
    auto func = mlir::func::FuncOp::create(rewriter, op.getLoc(), op.getName(),
                                           op.getFunctionType());

    rewriter.inlineRegionBefore(op.getRegion(), func.getBody(), func.end());
    rewriter.eraseOp(op);
    return success();
  }
};


struct ReturnOpLowering : public OpConversionPattern<mlir::typher::ReturnOp> {
    using OpConversionPattern<mlir::typher::ReturnOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(mlir::typher::ReturnOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const
    {
        rewriter.replaceOpWithNewOp<func::ReturnOp>(op, adaptor.getOperands());
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
