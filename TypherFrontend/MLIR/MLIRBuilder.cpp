#include "MLIRBuilder.h"


namespace MLIR {

    void Builder::BuildModule(SlabVector<AST::Statement*>& ASTTree)
    {
        gen_.Generate(ASTTree);
        emit_.Emit(gen_.Context(), gen_.Module());
    }
}
