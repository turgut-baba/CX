#include "MLIRBuilder.h"


namespace MLIR {

    void Builder::BuildModule(SlabVector<AST::Statement*>& ASTTree)
    {
        gen_.Generate(ASTTree);
    }
}
