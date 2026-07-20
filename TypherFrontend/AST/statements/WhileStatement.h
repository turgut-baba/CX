#ifndef WHILE_STATEMENT_H
#define WHILE_STATEMENT_H

#include "ASTNode.h"
#include "Expression.h"
#include "Statement.h"

namespace AST {
    class WhileStatement: public Statement {
    public:
        WhileStatement(Expression* expr, SlabAllocator* alloc);

        virtual std::string String() override
		{
			return "While statement ";
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