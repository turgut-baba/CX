#include "MLIREmitter.h"

#include "mlir/Dialect/Func/Extensions/AllExtensions.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/Transforms/InlinerInterfaceImpl.h"
#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

namespace MLIR{

    void Emitter::Emit()
    {
        mlir::PassManager pm(theModule->getName());

		//Lower to affine
		{		
			pm.addPass(mlir::createInlinerPass());
			mlir::OpPassManager &optPM = pm.nest<mlir::typher::FuncOp>();
			//optPM.addPass(mlir::toy::createShapeInferencePass());
			optPM.addPass(mlir::createCanonicalizerPass());
			optPM.addPass(mlir::createCSEPass());	
		}

		{
			//pm.addPass(mlir::toy::createLowerToAffinePass());
			mlir::OpPassManager &optPM = pm.nest<mlir::func::FuncOp>();
			optPM.addPass(mlir::createCanonicalizerPass());
			optPM.addPass(mlir::createCSEPass());	
		}

		// Lower to llvm
		{
			//pm.addPass(mlir::toy::createLowerToLLVMPass());
	    	pm.addPass(mlir::LLVM::createDIScopeForLLVMFuncOpPass());
		}
    }

    struct TypherToLLVMLoweringPass
    : public PassWrapper<TypherToLLVMLoweringPass, OperationPass<ModuleOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TypherToLLVMLoweringPass)
        StringRef getArgument() const override { return "typher-to-llvm"; }

        void getDependentDialects(DialectRegistry &registry) const override {
            registry.insert<LLVM::LLVMDialect, scf::SCFDialect>();
        }
        void runOnOperation() {

            LLVMConversionTarget target(getContext());
            target.addLegalOp<ModuleOp>();


            LLVMTypeConverter typeConverter(&getContext());

            RewritePatternSet patterns(&getContext());
            populateAffineToStdConversionPatterns(patterns);
            populateSCFToControlFlowConversionPatterns(patterns);
            mlir::arith::populateArithToLLVMConversionPatterns(typeConverter, patterns);
            populateFinalizeMemRefToLLVMConversionPatterns(typeConverter, patterns);
            cf::populateControlFlowToLLVMConversionPatterns(typeConverter, patterns);
            cf::populateAssertToLLVMConversionPattern(typeConverter, patterns);
            populateFuncToLLVMConversionPatterns(typeConverter, patterns);

            //patterns.add<PrintOpLowering>(&getContext());

            auto module = getOperation();
            if (failed(applyFullConversion(module, target, std::move(patterns))))
                signalPassFailure();
            }
    };

    struct TypherToAffineLoweringPass
    : public PassWrapper<TypherToAffineLoweringPass, OperationPass<ModuleOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TypherToAffineLoweringPass)
        StringRef getArgument() const override { return "typher-to-affine"; }

        void getDependentDialects(DialectRegistry &registry) const override {
            registry.insert<affine::AffineDialect, func::FuncDialect,
                            memref::MemRefDialect>();
        }
        void runOnOperation() 
        {
            ConversionTarget target(getContext());
            target.addLegalDialect<affine::AffineDialect, BuiltinDialect,
                                    arith::ArithDialect, func::FuncDialect,
                                    memref::MemRefDialect>();

            target.addIllegalDialect<typher::TypherDialect>();
            target.addDynamicallyLegalOp<typher::PrintOp>([](typher::PrintOp op) {
                return llvm::none_of(op->getOperandTypes(),
                                    [](Type type) { return llvm::isa<TensorType>(type); });
            });

            RewritePatternSet patterns(&getContext());
            patterns.add<AddOpLowering, ConstantOpLowering, FuncOpLowering, MulOpLowering,
                        PrintOpLowering, ReturnOpLowering, TransposeOpLowering>(
                &getContext());

            if (failed(
                    applyPartialConversion(getOperation(), target, std::move(patterns))))
                signalPassFailure();
        }
    };
    
    struct ShapeInferencePass
    : public mlir::PassWrapper<ShapeInferencePass, OperationPass<typher::FuncOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ShapeInferencePass)
        StringRef getArgument() const override { return "typher-shape-inference"; }

        void runOnOperation() override {
            auto f = getOperation();
            llvm::SmallPtrSet<mlir::Operation *, 16> opWorklist;
            f.walk([&](mlir::Operation *op) {
            if (returnsDynamicShape(op))
                opWorklist.insert(op);
            });


            while (!opWorklist.empty()) {
                auto nextop = llvm::find_if(opWorklist, allOperandsInferred);
                if (nextop == opWorklist.end())
                    break;

                Operation *op = *nextop;
                opWorklist.erase(op);

                // Ask the operation to infer its output shapes.
                LDBG() << "Inferring shape for: " << *op;
                if (auto shapeOp = dyn_cast<ShapeInference>(op)) {
                    shapeOp.inferShapes();
                } else {
                    op->emitError("unable to infer shape of operation without shape "
                                "inference interface");
                    return signalPassFailure();
                }
            }

            // If the operation worklist isn't empty, this indicates a failure.
            if (!opWorklist.empty()) {
                f.emitError("Shape inference failed, ")
                    << opWorklist.size() << " operations couldn't be inferred\n";
                signalPassFailure();
            }
        }

        /// A utility method that returns if the given operation has all of its
        /// operands inferred.
        static bool allOperandsInferred(Operation *op) {
            return llvm::all_of(op->getOperandTypes(), [](Type operandType) {
            return llvm::isa<RankedTensorType>(operandType);
            });
        }

        /// A utility method that returns if the given operation has a dynamically
        /// shaped result.
        static bool returnsDynamicShape(Operation *op) {
            return llvm::any_of(op->getResultTypes(), [](Type resultType) {
            return !llvm::isa<RankedTensorType>(resultType);
            });
        }
        };

    std::unique_ptr<mlir::Pass> Emitter::createShapeInferencePass() {
        return std::make_unique<ShapeInferencePass>();
    }

    std::unique_ptr<mlir::Pass> Emitter::createLowerToLLVMPass() {
        return std::make_unique<TypherToLLVMLoweringPass>();
    }

    std::unique_ptr<Pass> Emitter::createLowerToAffinePass() {
        return std::make_unique<TypherToAffineLoweringPass>();
    }

}