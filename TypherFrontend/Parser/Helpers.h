#ifndef HELPERS_H
#define HELPERS_H

#include "AST/ASTNode.h"

namespace Parser {
    static AstBuiltinTypes TokenTypeToAstType(Lex::TokenKeyword token_type)
    {
        switch(token_type) {
            case Lex::TokenKeyword::VOID:
                return AstBuiltinTypes::VOID;
            case Lex::TokenKeyword::BOOL:
                return AstBuiltinTypes::BOOL;
            case Lex::TokenKeyword::CHAR:
                return AstBuiltinTypes::CHAR;
            case Lex::TokenKeyword::INT:
                return AstBuiltinTypes::INT;
            case Lex::TokenKeyword::FLOAT:
                return AstBuiltinTypes::FLOAT;
            case Lex::TokenKeyword::DOUBLE:
                return AstBuiltinTypes::DOUBLE;
            default:
                return AstBuiltinTypes::NONE;
        }
    }
}

#endif