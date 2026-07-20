#include "statements/ForStatement.h"

namespace AST{
    ForStatement::ForStatement(Statement* init, Expression* cond, Expression* iter, SlabAllocator* alloc)
    {
        // TODO: check if the expr is compatible with the if statement
        body_ = alloc->Allocate<Body>(alloc);
        body_->SetOwner(this);

        initialize_ = init;
        initialize_->SetParent(this);
        
        condition_ = cond;
        condition_->SetParent(this);
        
        iterator_ = iter;
        iterator_->SetParent(this);
    }
}
