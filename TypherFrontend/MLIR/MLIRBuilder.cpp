#include "MLIRBuilder.h"


namespace MLIR {

    void Builder::BuildModule(ArrayAlloc<AST::Statement*>& ASTTree)
    {
        gen_.Generate(ASTTree);
        gen_.GenExe();
    }
}
