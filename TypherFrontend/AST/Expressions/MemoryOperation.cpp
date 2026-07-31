#include "MemoryOperation.h"

namespace AST {

MemoryOperation::MemoryOperation(AST::Expression* expr, unsigned int de_ref_depth, unsigned int address_depth)
    : Expression(AstNodeType::MEMORY_OPERATION), expr_(expr), de_ref_depth_(de_ref_depth), address_depth_(address_depth) 
{
    expr_->SetParent(this);
}

MemoryOperation::MemoryOperation(AST::Expression* expr, std::vector<AST::Expression*> indices, unsigned int array_depth)
    : Expression(AstNodeType::MEMORY_OPERATION), expr_(expr), array_indices_(indices), array_depth_(array_depth) 
{
    expr_->SetParent(this);
    for(auto& indices : array_indices_) {
        indices->SetParent(this);
    }
}

}