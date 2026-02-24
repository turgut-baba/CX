#include "AST/statements/IfStatement.h"

namespace AST{
    IfStatement::IfStatement(Expression* expr, SlabAllocator* alloc)
        :else_if_statements_(alloc)
    {
        // TODO: check if the expr is compatible with the if statement

        body_ = alloc->Allocate<Body>(alloc);
        body_->SetOwner(this);

        expr->SetParent(this);
        condition_ = expr;
    }
}