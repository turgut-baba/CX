#ifndef MLIR_HELPERS_H
#define MLIR_HELPERS_H

#include "mlir/IR/Builders.h"
#include "ASTNode.h"

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

    int64_t EvaluateCompileTimeConstant(AST::Expression* expr) {
        if (!expr) return 0;

        // 1. Base case: Literal integer (e.g. '3')
        if (expr->NodeType() == AstNodeType::LITERAL && static_cast<AST::Literal*>(expr)->IsIntegerLiteral()) {
            return static_cast<AST::IntegerLiteral*>(expr)->Value<int64_t>(); 
        }

        //if (auto* sizeOfExpr = llvm::dyn_cast<AST::SizeOfExpr>(expr)) {
        //    return getTypeSizeInBytes(sizeOfExpr->GetTargetType());
        //}

        return 1; // Fallback
    }

    void ApplyTypeModifiers(AST::VariableDeclarator* node, mlir::Type& varType, std::shared_ptr<mlir::OpBuilder> builder)
    {
        const auto& indexExprs = node->ArrayIndexExpressions();
        for (int i = static_cast<int>(indexExprs.size()) - 1; i >= 0; --i) {
            int64_t arraySize = EvaluateCompileTimeConstant(indexExprs[i]); 
            
            // Wrap current varType in ArrayType: !typher.array<N x varType>
            varType = mlir::typher::ArrayType::get(builder->getContext(), arraySize, varType);
        }

        // TODO: change this to be a better fit. no need for a for loop if
        // the only modifier is a pointer.
        for (auto& modifier: node->Modifiers()) {
			if (modifier == AST::DeclaratorKind::Pointer) {
        		varType = mlir::typher::PointerType::get(builder->getContext(), varType);
			}
    	}
    }
}

#endif