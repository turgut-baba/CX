#include "MLIREmitter.h"

#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "mlir/IR/BuiltinDialect.h" 
/* #include "toy/Dialect.h"
#include "toy/Passes.h" */
#include "mlir/Dialect/Affine/IR/AffineOps.h"

#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Casting.h"

#include "Dialect/DialectLowering.h"

namespace MLIR{

    std::unique_ptr<mlir::Pass> createShapeInferencePass();

    std::unique_ptr<mlir::Pass> createLowerToAffinePass();

    std::unique_ptr<mlir::Pass> createLowerToLLVMPass();

    using namespace mlir;

    void Emitter::Emit(mlir::MLIRContext &context, mlir::ModuleOp& module)
    {
/*         mlir::registerBuiltinDialectTranslation(*module->getContext());
		mlir::registerLLVMDialectTranslation(*module->getContext());

		llvm::LLVMContext llvmContext;
		auto llvmModule = mlir::translateModuleToLLVMIR(module, llvmContext);
         */
        mlir::PassManager pm(module->getContext());

        mlir::applyPassManagerCLOptions(pm);

		//Lower to affine


		{
			pm.addPass(createLowerToAffinePass());
			mlir::OpPassManager &optPM = pm.nest<mlir::func::FuncOp>();
			optPM.addPass(mlir::createCanonicalizerPass());
			optPM.addPass(mlir::createCSEPass());	
		}

		// Lower to llvm
		{
			pm.addPass(createLowerToLLVMPass());
	    	pm.addPass(mlir::LLVM::createDIScopeForLLVMFuncOpPass());
		}
        std::cout << "Runnin module" << std::endl;
        pm.run(module);
        module->dump();
        std::cout << "Done." << std::endl;
    }

    struct TypherToLLVMLoweringPass
    : public PassWrapper<TypherToLLVMLoweringPass, OperationPass<ModuleOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TypherToLLVMLoweringPass)
        StringRef getArgument() const override { return "typher-to-llvm"; }

        void getDependentDialects(DialectRegistry &registry) const override {
            registry.insert<LLVM::LLVMDialect, scf::SCFDialect>();
        }
        void runOnOperation() {
            std::cout << "Run on op" << std::endl;
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
            std::cout << "Run on op affine" << std::endl;
            LLVMTypeConverter typeConverter(&getContext());
            
            ConversionTarget target(getContext());

            target.addLegalDialect<affine::AffineDialect, BuiltinDialect,
                         arith::ArithDialect, func::FuncDialect,
                         memref::MemRefDialect>();

            target.addIllegalDialect<mlir::typher::TypherDialect>();
/*             target.addDynamicallyLegalOp<toy::PrintOp>([](toy::PrintOp op) {
                return llvm::none_of(op->getOperandTypes(),
                                    [](Type type) { return llvm::isa<TensorType>(type); });
            }); */

            
            RewritePatternSet patterns(&getContext());
            patterns.add<AddOpLowering, ConstantOpLowering, FuncOpLowering, ReturnOpLowering>(
                &getContext());

            std::cout << "Finished affine" << std::endl;
            if (failed(
                    applyPartialConversion(getOperation(), target, std::move(patterns))))
                signalPassFailure();
        }
    };

    std::unique_ptr<mlir::Pass> createLowerToLLVMPass() 
    {
        return std::make_unique<TypherToLLVMLoweringPass>();
    }

    std::unique_ptr<mlir::Pass> createLowerToAffinePass() 
    {
        return std::make_unique<TypherToAffineLoweringPass>();
    }

}