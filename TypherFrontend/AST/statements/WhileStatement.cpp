#include "statements/WhileStatement.h"

namespace AST{
    WhileStatement::WhileStatement(Expression* expr, SlabAllocator* alloc)
    {
        // TODO: check if the expr is compatible with the if statement

        body_ = alloc->Allocate<Body>(alloc);
        body_->SetOwner(this);

        expr->SetParent(this);
        condition_ = expr;
    }
}
