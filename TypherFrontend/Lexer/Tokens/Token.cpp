#include "Token.h"

namespace Lexer {

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

	template void Token::SetTokenType<TokenKeyword>(TokenKeyword);
	template void Token::SetTokenType<TokenOperator>(TokenOperator);
	template void Token::SetTokenType<TokenPunctuator>(TokenPunctuator);
	template void Token::SetTokenType<TokenLiteral>(TokenLiteral);
}