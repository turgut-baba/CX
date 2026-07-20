#ifndef FOR_STATEMENT_H
#define FOR_STATEMENT_H

#include "ASTNode.h"
#include "Expression.h"
#include "Statement.h"

namespace AST {
    class ForStatement: public Statement {
    public:
        ForStatement(Statement* init, Expression* cond, Expression* iter, SlabAllocator* alloc);

        virtual std::string String() override
		{
			return "For statement ";
		}

        Statement* InitializeStmt()
        {
            return initialize_;
        }

        Expression* ConditionExpr()
        {
            return condition_;
        }

        Expression* IteratorExpr()
        {
            return iterator_;
        }

        void Accept(NodeVisitor* v) override { v->Visit(this); }
    private:
        Statement* initialize_;
        Expression* condition_;
        Expression* iterator_;
    };

}
#endif