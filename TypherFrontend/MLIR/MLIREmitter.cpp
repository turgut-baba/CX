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
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Casting.h"

#include "Dialect/DialectLowering.h"

// For compilation
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/IR/LegacyPassManager.h"

namespace MLIR{

    std::unique_ptr<mlir::Pass> createShapeInferencePass();

    std::unique_ptr<mlir::Pass> createLowerToAffinePass();

    std::unique_ptr<mlir::Pass> createLowerToLLVMPass();

    void LowerToLLVMIR(mlir::ModuleOp module);

    using namespace mlir;

    void Emitter::Emit(mlir::MLIRContext &context, mlir::ModuleOp& module)
    {
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
        pm.run(module);
        LowerToLLVMIR(module);
        module->dump();
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
                         arith::ArithDialect, func::FuncDialect, cf::ControlFlowDialect,
                         memref::MemRefDialect>();

            target.addIllegalDialect<mlir::typher::TypherDialect>();
/*             target.addDynamicallyLegalOp<toy::PrintOp>([](toy::PrintOp op) {
                return llvm::none_of(op->getOperandTypes(),
                                    [](Type type) { return llvm::isa<TensorType>(type); });
            }); */

            
            RewritePatternSet patterns(&getContext());
            patterns.add<
                AddOpLowering, 
                CallOpLowering, 
                ConstantOpLowering, 
                FuncOpLowering, 
                ReturnOpLowering,
                IfOpLowering,
                EqualsOpLowering
                >(&getContext());

            if (failed(
                    applyPartialConversion(getOperation(), target, std::move(patterns))))
                signalPassFailure();
        }
    };

    void emitObjectFile(llvm::Module* llvmModule, llvm::TargetMachine* tm); // TEMP

    void LowerToLLVMIR(mlir::ModuleOp module)
    {
        // TRANSFORM TO LLVM IR
        mlir::registerBuiltinDialectTranslation(*module->getContext());
        mlir::registerLLVMDialectTranslation(*module->getContext());

        // Convert the module to LLVM IR in a new LLVM IR context.
        llvm::LLVMContext llvmContext;
        auto llvmModule = mlir::translateModuleToLLVMIR(module, llvmContext);
        if (!llvmModule) {
            llvm::errs() << "Failed to emit LLVM IR\n";
            return ;
        }

        // Initialize LLVM targets.
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();

        // Create target machine and configure the LLVM Module
        auto tmBuilderOrError = llvm::orc::JITTargetMachineBuilder::detectHost();
        if (!tmBuilderOrError) {
            llvm::errs() << "Could not create JITTargetMachineBuilder\n";
            return ;
        }

        auto tmOrError = tmBuilderOrError->createTargetMachine();
        if (!tmOrError) {
            llvm::errs() << "Could not create TargetMachine\n";
            return ;
        }

        mlir::ExecutionEngine::setupTargetTripleAndDataLayout(llvmModule.get(),
                                                                tmOrError.get().get());
        
        // GEN EXE
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();

        std::string TargetTripleStr = LLVMGetDefaultTargetTriple();
        llvm::Triple TargetTriple(TargetTripleStr);
        llvmModule->setTargetTriple(TargetTriple);

        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

        if (!Target) {
            llvm::errs() << "Error: " << Error;
            return ;
        }
        
        auto cpu = "generic";
        auto features = "";
        llvm::TargetOptions opt;
        auto rm = llvm::Reloc::Model::PIC_;

        // Pass the Triple object here
        llvm::TargetMachine* targetMachine = Target->createTargetMachine(
            TargetTriple, // This returns the string the API expects
            cpu, 
            features, 
            opt, 
            rm
        );

        llvmModule->setDataLayout(targetMachine->createDataLayout());
        emitObjectFile(llvmModule.get(), targetMachine);
    }

    int linkWithLLD(const std::string& objectFile, const std::string& outputFile);

    void emitObjectFile(llvm::Module* llvmModule, llvm::TargetMachine* tm)
    {
        std::string filename = "typher_output.o";
        std::error_code ec;
        llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

        if (ec) {
            llvm::errs() << "Could not open file: " << ec.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto fileType = llvm::CodeGenFileType::ObjectFile; // Use .AssemblyFile for .s
        
        if (tm->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
            llvm::errs() << "The TargetMachine can't emit a file of this type";
            return;
        }
        
        pass.run(*llvmModule);
        dest.flush();
        
        int res = linkWithLLD(filename, "Sanity");
        if(res) 
        {
            llvm::outs() << "Successfully compiled to " << filename << "\n";
        }
    }

    int linkWithLLD(const std::string& objectFile, const std::string& outputFile) {
        std::string command = "clang " + objectFile + " -o " + outputFile + " -lm"; 
    
        std::cout << "Linking with command: " << command << "..." << std::endl;
        
        int exitCode = std::system(command.c_str());
        
        if (exitCode != 0) {
            std::cerr << "Linking failed with exit code: " << exitCode << std::endl;
        }
        return exitCode;
    }

    std::unique_ptr<mlir::Pass> createLowerToLLVMPass() 
    {
        return std::make_unique<TypherToLLVMLoweringPass>();
    }

    std::unique_ptr<mlir::Pass> createLowerToAffinePass() 
    {
        return std::make_unique<TypherToAffineLoweringPass>();
    }

}