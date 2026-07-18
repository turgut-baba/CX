#ifndef RETURN_STATEMENT_H
#define RETURN_STATEMENT_H

#include "Statement.h"
#include "Expression.h"

namespace AST {
    class ReturnStatement: public Statement {
    public:
        ReturnStatement(){}
        
        ReturnStatement(Expression* expr);

        ReturnStatement(Function* func)
            : OwnerFunction_(func)
        {}

        ReturnStatement(Expression* expr,  Function* func);

        void SetParent(ASTNode* parent) override
        {
            parent_ = parent;
			parent->AddChildren(this);
            // TODO: assert parent type is function.
            OwnerFunction_ = (Function*)parent;
        }

        virtual std::string String() override
		{
			return "Return statement ";
		}

        Expression* Expr()
        {
            return ReturnedExpression_;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
    private:
        Expression* ReturnedExpression_;
        Function* OwnerFunction_;
};
}

#endif