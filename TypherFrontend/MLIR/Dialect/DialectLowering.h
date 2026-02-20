using namespace MLIR;

template <typename BinaryOp, typename LoweredBinaryOp>
struct BinaryOpLowering : public OpConversionPattern<BinaryOp> {
    using OpConversionPattern<BinaryOp>::OpConversionPattern;
    using OpAdaptor = typename OpConversionPattern<BinaryOp>::OpAdaptor;

    LogicalResult matchAndRewrite(BinaryOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const {
        std::cout << "Lowerin bin" << std::endl;
        auto loc = op->getLoc();
        Value lhs = adaptor.getLhs();
        Value rhs = adaptor.getRhs();

        if (!llvm::isa<IntegerType>(lhs.getType())) {
            return rewriter.notifyMatchFailure(op, "Expected scalar integer types.");
        }

        auto scalarResult = rewriter.create<arith::AddIOp>(loc, lhs, rhs);

        rewriter.replaceOp(op, scalarResult.getResult());
        std::cout << "Finished bin" << std::endl;
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
        std::cout << "Finished const" << std::endl;
        return success();
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
                  ConversionPatternRewriter &rewriter) const {
    rewriter.replaceOpWithNewOp<func::ReturnOp>(op, adaptor.getOperands());
    return success();
  }
};
