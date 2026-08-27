#include "SemanticAnalyzer.h"

namespace checker {
    void SemanticAnalyzer::Check(AST::VariableDeclaration *node)
    {
        AstBuiltinTypes type = node->Type();

        for(auto declarator: node->Declarators()) {
            AST::Expression* rhs = declarator->Expr();
            Check(rhs);
            SymbolTable()->declare(declarator->Ident(), node->Type());
        }

        if(node->NodeType() == AstNodeType::VARIABLE_DECLARATOR) {
            SymbolTable()->declare(node->Ident(), node->Type());
        } else if(node->NodeType() == AstNodeType::IDENTIFIER) {
            bool found = SymbolTable()->lookup(node->Ident());
            if(!found) {
                // throw undefined reference error
            }
        }
    }

    /*checker::Type*/ void SemanticAnalyzer::Check(AST::Identifier *node)
    {

    }
    
}
