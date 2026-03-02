#ifndef WHILE_STATEMENT_H
#define WHILE_STATEMENT_H

#include "AST/ASTNode.h"
#include "AST/Expression.h"
#include "AST/Statement.h"

namespace AST {
    class WhileStatement: public Statement {
    public:
        WhileStatement(Expression* expr, SlabAllocator* alloc);

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
        Body* else_body_ = nullptr;
        WhileStatement* elif_statement_ = nullptr;
    };

}
#endif