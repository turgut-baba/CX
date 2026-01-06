#include "ExpressionParser.h"

namespace Parser {
	AST::CallExpression* ExpressionParser::ParseFunctionCall(AST::Identifier* ident)
	{
		return nullptr;
	}


	AST::ASTNode* ExpressionParser::CheckIdentifier()
	{
		AST::Identifier* ident = new AST::Identifier(Lexer()->GetToken().Ident());
		Lexer()->NextToken();

		if (Lexer()->GetToken().IsTokenType<Lex::TokenOperator>(Lex::TokenOperator::LEFT_PAREN)) {
			AST::CallExpression* callExpr = ParseFunctionCall(ident);
			return callExpr;
		}
		else if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::DOT)) {
			// TODO: Handle class members.
		}

		return ident;

	}
	AST::Expression* ExpressionParser::parse_expression() 
	{
		AST::Identifier* idents;
		AST::Identifier* literals;
		AST::Operator* operator_;

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_PARENTHESES) ) {
			Lexer()->NextToken();
			auto parenExpr = parse_expression();
			parenExpr->SetParent(operator_);
			Lexer()->NextToken(); // Skip ')'
		} else {
			auto ident = CheckIdentifier();//new AST::Identifier(Lexer()->GetToken().Ident());
			ident->SetParent(operator_);
			idents.add(ident);
		}

		if (Lexer()->GetToken().Type() != Lex::TokenType::Operator) {
			//TODO:: raise error
		}

		operator_ = AST::Operator(Lexer()->GetToken());

		Lexer()->NextToken();

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			auto parenExpr = parse_expression();
			parenExpr->SetParent(operator_);
		}
		else {
			auto ident = CheckIdentifier();//new AST::Identifier(Lexer()->GetToken().Ident());
			ident->SetParent(operator_);
			idents.add(ident);
		}

		return operator_;

	}
};