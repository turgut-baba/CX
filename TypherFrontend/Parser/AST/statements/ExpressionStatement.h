#ifndef EXPRESSION_STATEMENT_H
#define EXPRESSION_STATEMENT_H

#include "AST/Statement.h"
#include "AST/Expression.h"

namespace AST {
    class ExpressionStatement: public Statement {
    public:
        ExpressionStatement(Expression* expr);

        virtual std::string String() override
		{
			return "Expression statement ";
		}
        
        void Accept(NodeVisitor* v) override { v->Visit(this); }

        Expression* Expr()
        {
            return expression_;
        }
    private:
        Expression* expression_;
    };
}

#endif