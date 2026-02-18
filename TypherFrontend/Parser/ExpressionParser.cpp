#include "ExpressionParser.h"

namespace Parser {
	AST::CallExpression* ExpressionParser::ParseFunctionCall(AST::Identifier* ident)
	{
		SlabVector<AST::Expression*> params(Allocator());
		Lexer()->NextToken();
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_PARENTHESES)) {
			params.push_back(parse_expression());
			if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
				Lexer()->NextToken(); // Skip ','
			}
		}
		Lexer()->NextToken(); // Skip ')'

		return Allocator()->Allocate<AST::CallExpression>(ident->Value(), params);
	}


	AST::ASTNode* ExpressionParser::CheckIdentifier()
	{
		AST::Identifier* ident = Allocator()->Allocate<AST::Identifier>(Lexer()->GetToken().Ident());
		state_->diags.report<DiagLevel::Message>({}) 
            	<< "Log: l:" << Lexer()->GetToken().GetLocation().line << " c: " << Lexer()->GetToken().GetLocation().col
				<< " Ident: "<< Lexer()->GetToken().Ident();
		ident->SetLocation(Lexer()->GetToken().GetLocation());
		Lexer()->NextToken();

		if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			AST::CallExpression* callExpr = ParseFunctionCall(ident);
			return callExpr;
		}
		else if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::DOT)) {
			// TODO: Handle class members.
		}

		return ident;
	}

	AST::Literal* ExpressionParser::CheckLiteral()
	{
		switch (Lexer()->GetToken().GetTokenType<Lex::TokenLiteral>()) {
		case Lex::TokenLiteral::DECIMAL: {
			int number = stoi(Lexer()->GetToken().Ident());
			AST::IntegerLiteral* ident = Allocator()->Allocate<AST::IntegerLiteral>(number);
			ident->SetLocation(Lexer()->GetToken().GetLocation());
			return ident;
		}

		}
		return nullptr;
	}

	AST::Expression* ExpressionParser::ParseAdditiveExpression()
	{
		AST::ASTNode* lhs;

		if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			// Check if the lhs is an expression itself with () for example: (a + 5) + b
			Lexer()->NextToken();// Skip '('
			lhs = parse_expression();
			Lexer()->NextToken(); // Skip ')'
		} else if (Lexer()->GetToken().Type() == Lex::TokenType::Literal) {
			// Check individual literals for example: 5 + b
			lhs = CheckLiteral();
			Lexer()->NextToken();
		} else {		
			// Check individual identifiers for example: a + b
			lhs = CheckIdentifier();
		}

		if (IsStatementEnd()) {
			return (AST::Expression*)lhs;
		}

		if (Lexer()->GetToken().Type() != Lex::TokenType::Operator) {	
			state_->diags.report<DiagLevel::Error>({}) 
            	<< "Unexpected token. Expected ';' or an operator.";
		}

		const auto tokenType = Lexer()->GetToken().GetTokenType<Lex::TokenOperator>();
		AST::Operator* operator_ = Allocator()->Allocate<AST::Operator>(tokenType);

		lhs->SetParent(operator_);

		Lexer()->NextToken();

		AST::ASTNode* rhs;
		if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			Lexer()->NextToken();// Skip '('
			rhs = parse_expression();
			Lexer()->NextToken(); // Skip ')'		
		} else if (Lexer()->GetToken().Type() == Lex::TokenType::Literal) {
			
			rhs = CheckLiteral();
			Lexer()->NextToken();

		} else {
			rhs = CheckIdentifier();
		}
		
		rhs->SetParent(operator_);

		operator_->SetLHS(lhs);
		operator_->SetRHS(rhs);

		return operator_;
	}

	AST::Expression* ExpressionParser::parse_expression() 
	{
		return ParseAdditiveExpression();
	}



	AST::Expression* ExpressionParser::parse_assignment()
	{
		// This handles 'a = b;' basically a re-assignment.
		return nullptr;
	}
};