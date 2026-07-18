#ifndef HELPERS_H
#define HELPERS_H

#include "ASTNode.h"

namespace Parser {
    static AstBuiltinTypes TokenTypeToBuiltinAstType(Lex::TokenKeyword token_type)
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

    static std::string AstTypeToString(AstBuiltinTypes AST_type)
    {
        switch(AST_type) {
            case AstBuiltinTypes::VOID:
                return "void";
            case AstBuiltinTypes::BOOL:
                return "bool";
            case AstBuiltinTypes::CHAR:
                return "char";
            case AstBuiltinTypes::INT:
                return "int";
            case AstBuiltinTypes::FLOAT:
                return "float";
            case AstBuiltinTypes::DOUBLE:
                return "double";
            default:
                return "N/A";
        }
    }
}

#endif