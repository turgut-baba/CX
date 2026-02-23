#ifndef IF_STATEMENT_H
#define IF_STATEMENT_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"
#include "AST/Statement.h"

namespace AST {
    class IfStatement: public Statement {
    public:
        IfStatement(Expression* expr)
        {
            // TODO: check if the expr is compatible with the if statement
            expr->SetParent(this);
        }

        virtual std::string String() override
		{
			return "If statement ";
		}

        Expression* ConditionExpr()
        {
            return condition_;
        }
    private:
        Expression* condition_;
        SlabVector<Statement*> body_;
    };

}
#endif