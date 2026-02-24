#ifndef IF_STATEMENT_H
#define IF_STATEMENT_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"
#include "AST/Statement.h"

namespace AST {
    class IfStatement: public Statement {
    public:
        IfStatement(Expression* expr, SlabAllocator* alloc);

        virtual std::string String() override
		{
			return "If statement ";
		}

        Expression* ConditionExpr()
        {
            return condition_;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
    private:
        Expression* condition_;
    };

}
#endif