#include "DeclaratorParser.hpp"
#include "ExpressionParser.h"
#include "StatementParser.h"

#include "Function.h"

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

    AST::VariableDeclarator* declarator = Allocator()->Allocate<AST::VariableDeclarator>(Ident, modifiers);
    
    auto token = Lexer()->GetToken();
    switch (token.Type()) {
        case Lex::TokenType::Punctuator:
        {
            while (token.IsTokenType(Lex::TokenPunctuator::LEFT_SQUARE_BRACKETS)) {
                std::cout << "Parsing array declarator for variable: " << declarator->Name() << std::endl;
                declarator->ToggleArrayDeclarator(Allocator());

                Lexer()->NextToken(); // Skip '['

                AST::Expression* index_expr = state_->expression_parser->parse_expression();

                declarator->AddArrayIndexExpression(index_expr);
                if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_SQUARE_BRACKETS)) {
                    Diagnostic().report<DiagLevel::Error>({}) 
                        << "Expected ']' after array declarator. Got: " << Lexer()->GetToken().Ident();
                }
                
                Lexer()->NextToken(); // Skip ']'

                token = Lexer()->GetToken();
            }
            [[fallthrough]];
        }
        case Lex::TokenType::Operator:
        {
            if (token.IsTokenType(Lex::TokenOperator::ASSIGNMENT)) {
                Lexer()->NextToken(); // Skip '='
                auto assigned_expression = state_->expression_parser->parse_expression();
                declarator->SetExpression(assigned_expression);
            }
            break;
        }
    }
    
    return declarator;
}

AST::Statement* DeclaratorParser::VariableOrFunctionDecl()
{
    // AST::Statement* statement = state_->assignment_parser->parse_assignment();
    AstBuiltinTypes type = ParseType();

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

void DeclaratorParser::ParseParameter(AST::Function* functionDecl)
{
    AstBuiltinTypes type = ParseType();

    Lexer()->NextToken(); // Skip the type

    AST::VariableDeclarator* param = parse_declarator();

    auto decl = Allocator()->Allocate<AST::VariableDeclaration>(param, Allocator(), type);

    functionDecl->AddParameter(decl);
}

AST::Statement* DeclaratorParser::ParseFunctionDeclaration(AST::VariableDeclarator* declarator, AstBuiltinTypes return_type)
{
    Lexer()->NextToken(); // Skip '('
    AST::Function* functionDecl = Allocator()->Allocate<AST::Function>(declarator, Allocator());
    while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_PARENTHESES)) {
        ParseParameter(functionDecl);

        if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
            Lexer()->NextToken(); // Skip ','
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