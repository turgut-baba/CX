#include "DeclaratorParser.hpp"
#include "ExpressionParser.h"
#include "StatementParser.h"

#include "AST/Function.h"

namespace Parser {
    
AST::VariableDeclarator* DeclaratorParser::parse_declarator() 
{
    std::vector<AST::DeclaratorKind> modifiers;
    AST::Identifier* Ident;
    bool parse_modifiers = true;
    do {
        auto token = Lexer()->GetToken();
        switch (token.Type()) {
            case Lex::TokenType::Operator:
                if(token.IsTokenType(Lex::TokenOperator::BITWISE_AND)) {
                    modifiers.push_back(AST::DeclaratorKind::Reference);
                    Lexer()->NextToken();
                }
                if (token.IsTokenType(Lex::TokenOperator::MULTIPLY)) {
                    modifiers.push_back(AST::DeclaratorKind::Pointer);
                    Lexer()->NextToken();
                }
                break;
            case Lex::TokenType::Identifier:
            {
                Ident = Allocator()->Allocate<AST::Identifier>(token.Ident());
                parse_modifiers = false;
                break;
            }
        }
    } while (parse_modifiers);

    Lexer()->NextToken(); // Skip ident
    auto token = Lexer()->GetToken();
    switch (token.Type()) {
        case Lex::TokenType::Punctuator:
        {
            if (token.IsTokenType(Lex::TokenPunctuator::LEFT_SQUARE_BRACKETS)) {
                // TODO: Handle arrays
            }
            break;
        }
        case Lex::TokenType::Operator:
        {
            if (token.IsTokenType(Lex::TokenOperator::ASSIGNMENT)) {
                Lexer()->NextToken(); // Skip '='
                auto assigned_expression = state_->expression_parser->parse_expression();
                return Allocator()->Allocate<AST::VariableDeclarator>(assigned_expression, Ident, modifiers);
            }
        }
    }
    
    return Allocator()->Allocate<AST::VariableDeclarator>(Ident, modifiers);
}

AST::Statement* DeclaratorParser::VariableOrFunctionDecl()
{
    // AST::Statement* statement = state_->assignment_parser->parse_assignment();

    Lex::TokenKeyword token_style_type = Lexer()->GetToken().GetTokenType<Lex::TokenKeyword>(); // TODO: Turn this into a type class.
    AstBuiltinTypes type = TokenTypeToAstType(token_style_type);

    Lexer()->NextToken(); // Skip the type
    SlabVector<AST::VariableDeclarator*> declarators = nullptr;

    AST::VariableDeclarator* declarator = parse_declarator();

    auto token = Lexer()->GetToken();
    if (token.IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
        return ParseFunctionDeclaration(declarator, type);
    }
    while(true) {
        token = Lexer()->GetToken();
        switch (token.Type()) {
            case Lex::TokenType::Punctuator:
            {
                if (token.IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
                    Lexer()->NextToken();
                    if (declarators.Initialized()) {
                        return Allocator()->Allocate<AST::VariableDeclaration>(declarator, Allocator(), type);
                    } else {
                        return Allocator()->Allocate<AST::VariableDeclaration>(declarators, type);

                    }
                } else if (token.IsTokenType(Lex::TokenPunctuator::COMMA)) {
                    if (!declarators.Initialized()) { 
                        declarators = Allocator()->ArrayAllocate<AST::VariableDeclarator*>();
                    }
                    declarators.push_back(declarator);
                    declarator = parse_declarator();
                    break;
                }
                Diagnostic().report<DiagLevel::Error>({}) 
				    << "Unexpected token.";
            }
        }
        UNREACHABLE("Error with declaration.");
    }

    return nullptr;
}


AST::Statement* DeclaratorParser::ParseFunctionDeclaration(AST::VariableDeclarator* declarator, AstBuiltinTypes return_type)
{
    Lexer()->NextToken(); // Skip '('
    AST::Function* functionDecl = Allocator()->Allocate<AST::Function>(declarator, Allocator());
    while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_PARENTHESES)) {
        // ParseParameter();
        if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
            // Log error
        }
    }
    Lexer()->NextToken(); // Skip ')'

    if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::SEMICOLON)) {
        // return;
    }

    AST::Body* function_body = state_->statement_parser->ParseBody(functionDecl);
    functionDecl->SetBody(function_body);
    functionDecl->SetReturnType(return_type);

    return functionDecl;
}

}