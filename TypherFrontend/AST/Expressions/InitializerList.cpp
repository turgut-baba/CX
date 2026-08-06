#include "InitializerList.h"

namespace AST {
    InitializerList::InitializerList(SlabVector<Expression*> elements)
        : Expression(AstNodeType::INITIALIZER_LIST), elements_(elements) 
    {
        for(auto& element : elements_) {
            element->SetParent(this);
        }
    }

    
}