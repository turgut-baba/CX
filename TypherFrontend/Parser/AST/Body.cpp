#include "Body.h"
#include "AST/Statement.h"

namespace AST{
    Body::Body(SlabAllocator* alloc) 
        :statements_(alloc) { }

    Body::Body(SlabVector<Statement*> statements)
            :statements_(statements) { }

    void Body::SetOwner(Statement* owner)
    {
        owner_ = owner;
        for(auto statement: statements_)
        {
            statement->SetParent(owner);
        }
    }
}