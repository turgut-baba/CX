#include "Token.h"

namespace Lex {

	Token::Token() 
	{
		this->punctuator_type = TokenPunctuator::NOT_A_PUNCTUATOR;
		this->operator_type = TokenOperator::NOT_AN_OPERATOR;
		this->keyword_type = TokenKeyword::NOT_A_KEYWORD;
		this->literal_type = TokenLiteral::NOT_A_LITERAL;
	}

	template<typename TokenElavType>
	void Token::SetTokenType(TokenElavType type) {
		if constexpr (std::is_same_v<TokenElavType, TokenKeyword>) {
			SetType(TokenType::Keyword);
			this->keyword_type = type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenOperator>) {
			SetType(TokenType::Operator);
			this->operator_type = type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenPunctuator>) {
			SetType(TokenType::Punctuator);
			this->punctuator_type = type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenLiteral>) {
			SetType(TokenType::Literal);
			this->literal_type = type;
		} else {
			// TODO: Handle unreachable	
		}
	}

	template <typename E>
	void Zero(E& e) {
		e = static_cast<E>(0);
	}

	void Token::RestTokenTypes()
	{
		Zero(punctuator_type); 
		Zero(operator_type);
		Zero(keyword_type);
		Zero(literal_type);
	}


	template<typename TokenElavType>
	TokenElavType Token::GetTokenType() {
		if constexpr (std::is_same_v<TokenElavType, TokenKeyword>) {
			return this->keyword_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenOperator>) {
			return this->operator_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenPunctuator>) {
			return this->punctuator_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenLiteral>) {
			return this->literal_type;
		}
		else {
			// TODO: Handle unreachable	
		}
	}

	template<typename TokenElavType>
	bool Token::IsTokenType(TokenElavType type) {
		TokenElavType actual_type;
		if constexpr (std::is_same_v<TokenElavType, TokenKeyword>) {
			actual_type = this->keyword_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenOperator>) {
			actual_type = this->operator_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenPunctuator>) {
			actual_type = this->punctuator_type;
		}
		else if constexpr (std::is_same_v<TokenElavType, TokenLiteral>) {
			actual_type = this->literal_type;
		}
		else {
			// TODO: Handle unreachable	
		}

		return actual_type == type;
	}

	// TODO: Turn these into pre-processor macros
	template void Token::SetTokenType<TokenKeyword>(TokenKeyword);
	template void Token::SetTokenType<TokenOperator>(TokenOperator);
	template void Token::SetTokenType<TokenPunctuator>(TokenPunctuator);
	template void Token::SetTokenType<TokenLiteral>(TokenLiteral);

	template TokenKeyword Token::GetTokenType<TokenKeyword>();
	template TokenOperator Token::GetTokenType<TokenOperator>();
	template TokenPunctuator Token::GetTokenType<TokenPunctuator>();
	template TokenLiteral Token::GetTokenType<TokenLiteral>();

	template bool Token::IsTokenType<TokenKeyword>(TokenKeyword);
	template bool Token::IsTokenType<TokenOperator>(TokenOperator);
	template bool Token::IsTokenType<TokenPunctuator>(TokenPunctuator);
	template bool Token::IsTokenType<TokenLiteral>(TokenLiteral);
}