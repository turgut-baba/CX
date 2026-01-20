#include "ExpressionParser.h"

namespace Parser {
	AST::CallExpression* ExpressionParser::ParseFunctionCall(AST::Identifier* ident)
	{
		return nullptr;
	}


	AST::ASTNode* ExpressionParser::CheckIdentifier()
	{
		AST::Identifier* ident = Allocator()->Allocate<AST::Identifier>(Lexer()->GetToken().Ident());
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

		AST::ASTNode* lhs;
		AST::ASTNode* rhs;

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_PARENTHESES) ) {
			Lexer()->NextToken();
			lhs = parse_expression();
			Lexer()->NextToken(); // Skip ')'
		} else {
			lhs = CheckIdentifier();
		}

		if (Lexer()->GetToken().Type() != Lex::TokenType::Operator) {
			//TODO:: raise error
		}

		operator_ = new AST::Operator(Lexer()->GetToken().GetTokenType<Lex::TokenOperator>());

		lhs->SetParent(operator_);

		Lexer()->NextToken();

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			rhs = parse_expression();
		}
		else {
			rhs = CheckIdentifier();
		}
		
		rhs->SetParent(operator_);

		operator_->SetLHS(lhs);
		operator_->SetRHS(rhs);

		return operator_;

	}
};