#include "MLIRBuilder.h"


namespace MLIR {

    void Builder::BuildModule(SlabVector<AST::Statement*>& AST_tree)
    {
        gen_.Generate(AST_tree);
        emit_.Emit(gen_.Context(), gen_.Module());
    }
    
}

