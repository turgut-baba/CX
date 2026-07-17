#ifndef MLIR_HELPERS_H
#define MLIR_HELPERS_H

#include "mlir/IR/Builders.h"
#include "AST/ASTNode.h"

namespace MLIR {
    mlir::Type ASTTypeToMlirType(AstBuiltinTypes AST_type, std::shared_ptr<mlir::OpBuilder> builder)
    {
        switch(AST_type) {
            case AstBuiltinTypes::VOID:
                return builder->getI1Type();
            case AstBuiltinTypes::BOOL:
                return builder->getI1Type();
            case AstBuiltinTypes::CHAR:
                return builder->getI8Type();
            case AstBuiltinTypes::INT:
                return builder->getI32Type();
            case AstBuiltinTypes::FLOAT:
                return builder->getF32Type();
            case AstBuiltinTypes::DOUBLE:
                return builder->getF64Type();
            default:
                return builder->getI32Type();
        }
    }

    void ApplyTypeModifiers(AST::VariableDeclarator* node, mlir::Type& varType, std::shared_ptr<mlir::OpBuilder> builder)
    {
        for (auto& modifier: node->Modifiers()) {
			if(modifier == AST::DeclaratorKind::Pointer){
        		varType = mlir::typher::PointerType::get(builder->getContext(), varType);
			}
    	}
    }
}

#endif