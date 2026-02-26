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

        void SetElif(IfStatement* elif)
        {
            elif_statement_ = elif;
        }

        void SetElse(Body* els)
        {
            else_body_ = els;
        }

        bool HasElif()
        {
            return elif_statement_ != nullptr;
        }

        bool HasElse()
        {
            return else_body_ != nullptr;
        }

        Body* ElseBody()
        {
            return else_body_;
        }

        IfStatement* Elif()
        {
            return elif_statement_;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
    private:
        Expression* condition_;
        Body* else_body_ = nullptr;
        IfStatement* elif_statement_ = nullptr;
    };

}
#endif