#include "SemanticAnalyzer.h"

namespace checker {
    void SemanticAnalyzer::Check(AST::ASTNode *node)
    {
    }

    /*checker::Type*/ void SemanticAnalyzer::Check(AST::IntegerLiteral *node)
    {
        if(node->Parent()->NodeType() == AstNodeType::VARIABLE_DECLARATOR)
        {

        } else if(node->Parent()->NodeType() == AstNodeType::OPERATOR && 
                ((AST::Operator*)node->Parent())->OperatorType() == AST::OperatorKind::ASN) {
            
        }
    }
    
}
