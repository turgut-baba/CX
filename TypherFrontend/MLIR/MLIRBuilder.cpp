#include "MLIRBuilder.h"

namespace MLIR {

    void Builder::BuildModule() {
        mlir::registerAsmPrinterCLOptions();
        mlir::registerMLIRContextCLOptions();
        mlir::MLIRContext context;
        

        context.loadDialect<mlir::arith::ArithDialect, 
                        mlir::func::FuncDialect, 
                        mlir::BuiltinDialect>();
    }
}
