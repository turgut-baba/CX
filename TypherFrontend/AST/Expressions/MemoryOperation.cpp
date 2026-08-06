#include "MemoryOperation.h"

namespace AST {

MemoryOperation::MemoryOperation(Expression* expr, unsigned int de_ref_depth, unsigned int address_depth)
    : Expression(AstNodeType::MEMORY_OPERATION), expr_(expr), de_ref_depth_(de_ref_depth), address_depth_(address_depth) 
{
    expr_->SetParent(this);
}

MemoryOperation::MemoryOperation(Expression* expr, SlabAllocator *allocator, unsigned int array_depth_)
    : Expression(AstNodeType::MEMORY_OPERATION), expr_(expr), array_depth_(array_depth_) 
{
    array_indices_ = SlabVector<Expression*>(allocator);
    expr_->SetParent(this);
}

}