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

        void AddElif(IfStatement* elif)
        {
            else_if_statements_.push_back(elif);
        }

        void SetElse(Body* els)
        {
            else_body_ = els;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
    private:
        Expression* condition_;
        Body* else_body_ = nullptr;
        SlabVector<IfStatement*> else_if_statements_;
    };

}
#endif