#include "ExpressionParser.h"

namespace Parser {
	AST::CallExpression* ExpressionParser::ParseFunctionCall(AST::Identifier* ident)
	{
		SlabVector<AST::Expression*> params(Allocator());
		Lexer()->NextToken(); // Skip '('
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_PARENTHESES)) {
			params.push_back(parse_expression());
			if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
				Lexer()->NextToken(); // Skip ','
			}
		}
		Lexer()->NextToken(); // Skip ')'

		return Allocator()->Allocate<AST::CallExpression>(ident->Value(), params);
	}

	AST::MemoryOperation* ExpressionParser::ParseArray(AST::Expression* expr)
	{
		AST::MemoryOperation* MemOp = Allocator()->Allocate<AST::MemoryOperation>(expr, Allocator());
		while (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::LEFT_SQUARE_BRACKETS)) {
			Lexer()->NextToken(); // Skip '['
			AST::Expression* index_expr = parse_expression();
			MemOp->AddArrayIndexExpression(index_expr);
			if (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_SQUARE_BRACKETS)) {
				Diagnostic().report<DiagLevel::Error>({}) 
					<< "Expected ']' after array index expression.";
			}
			Lexer()->NextToken(); // Skip ']'
		}

		return MemOp;
	}

	AST::Expression* ExpressionParser::CheckIdentifier()
	{
		AST::Identifier* ident = Allocator()->Allocate<AST::Identifier>(Lexer()->GetToken().Ident());
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
				int number = Lexer()->GetToken().DecimalValue();
				AST::IntegerLiteral* ident = Allocator()->Allocate<AST::IntegerLiteral>(number);
				ident->SetLocation(Lexer()->GetToken().GetLocation());
				ident->SetType(false); // TODO: temp
				Lexer()->NextToken();
				return ident;
			}
			case Lex::TokenLiteral::FLOATING_PONINT: {
				float number = Lexer()->GetToken().FloatingValue();
				AST::IntegerLiteral* ident = Allocator()->Allocate<AST::IntegerLiteral>(number);
				ident->SetLocation(Lexer()->GetToken().GetLocation());
				ident->SetType(true); // TODO: temp
				Lexer()->NextToken();
				// TODO: check for the 'f' suffix for float literals and handle it.
				return ident;
			}
			case Lex::TokenLiteral::CHARACTER: {
				char number = (char)Lexer()->GetToken().DecimalValue();
				AST::StringLiteral* ident = Allocator()->Allocate<AST::StringLiteral>(number);
				ident->SetLocation(Lexer()->GetToken().GetLocation());
				ident->SetType(true); // TODO: temp
				Lexer()->NextToken();
				return ident;
			}
			case Lex::TokenLiteral::STRING: {
				const char* number = Lexer()->GetToken().Ident().c_str();
				AST::StringLiteral* ident = Allocator()->Allocate<AST::StringLiteral>((char*)number);
				ident->SetLocation(Lexer()->GetToken().GetLocation());
				ident->SetType(true); // TODO: temp
				Lexer()->NextToken();
				return ident;
			}
		}
		return nullptr;
	}

	AST::Expression* ExpressionParser::ParseOperator(AST::Expression* lhs)
	{
		const auto tokenType = Lexer()->GetToken().GetTokenType<Lex::TokenOperator>();
		AST::Operator* operator_ = Allocator()->Allocate<AST::Operator>(tokenType);
		
		lhs->SetParent(operator_);
		
		Lexer()->NextToken(); // Skip the operator (don't need expected token here)
		
		AST::Expression* rhs = parse_expression();
		
		rhs->SetParent(operator_);

		operator_->SetLHS(lhs);
		operator_->SetRHS(rhs);

		return operator_;
	}

	AST::Expression* ExpressionParser::ParseSingleExpression()
	{
		AST::Expression* expr;
		if (Lexer()->GetToken().Type() == Lex::TokenType::Literal) {
			expr = CheckLiteral();
		} else if (Lexer()->GetToken().Type() == Lex::TokenType::Identifier) {	
			expr = CheckIdentifier();
		} else if(Lexer()->GetToken().Type() == Lex::TokenType::Operator) {
			// TODO: also look for operators for Unary expressions. And some keywords
			// like true or nullptr.
			expr = CheckMemoryOperation();
		} else {
			Diagnostic().report<DiagLevel::Error>({}) 
				<< "Unexpected token '" << Lexer()->GetToken().Ident() << "'. Expected an operator.";
		}
		return expr;
	}

	AST::Expression* ExpressionParser::CheckMemoryOperation() 
	{
		unsigned int de_ref_depth = 0;
		unsigned int address_depth = 0;
		while (Lexer()->GetToken().IsTokenType(Lex::TokenOperator::MULTIPLY)) {
			Lexer()->NextToken(); // Skip '*'
			de_ref_depth++;
		}

		if ((Lexer()->GetToken().IsTokenType(Lex::TokenOperator::BITWISE_AND))) {
			Lexer()->NextToken(); // Skip '&'
			address_depth++;
		}

		auto* expr = parse_expression(); // Parse the accessed expression

		AST::MemoryOperation* memory_op = Allocator()->Allocate<AST::MemoryOperation>(expr, de_ref_depth, address_depth);
		return memory_op;
	}

	AST::InitializerList* ExpressionParser::ParseInitializerList(AST::Expression* expr)
	{
		Lexer()->NextToken(); // Skip '{'
		SlabVector<AST::Expression*> elements(Allocator());
		while (!Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::RIGHT_CURLY_BRACE)) {
			AST::Expression* element_expr = parse_expression();
			elements.push_back(element_expr);
			if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::COMMA)) {
				Lexer()->NextToken(); // Skip ','
			}

			// TODO: Handle the case where the initializer list is not properly closed with a '}'.
		}
		Lexer()->NextToken(); // Skip '}'

		return Allocator()->Allocate<AST::InitializerList>(elements);
	}

	AST::Expression* ExpressionParser::parse_expression() 
	{
		AST::Expression* expr;

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_CURLY_BRACE)) {
			return ParseInitializerList(expr);
		}

		if (Lexer()->GetToken().IsTokenType(Lex::TokenPunctuator::LEFT_PARENTHESES)) {
			// Check if the lhs is an expression itself with () for example: (a + 5) + b
			Lexer()->NextToken();// Skip '('
			expr = parse_expression();
			Lexer()->NextToken(); // Skip ')'
		} else {
			expr = ParseSingleExpression();
		}

		if(Lexer()->GetToken().Type() == Lex::TokenType::Operator) {
			return ParseOperator(expr);
		}

		if (Lexer()->GetToken().IsTokenType<Lex::TokenPunctuator>(Lex::TokenPunctuator::LEFT_SQUARE_BRACKETS)) {
			return ParseArray(expr);
		}

		return expr;

		//Diagnostic().report<DiagLevel::Error>({}) 
        //    << "Expected ';' at the end of expression.";
	}

	AST::Expression* ExpressionParser::parse_assignment()
	{
		// This handles 'a = b;' basically a re-assignment.
		return nullptr;
	}
};