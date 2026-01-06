#ifndef TOKEN_H
#define TOKEN_H
#include "TokenTypes.h"
#include <iostream>
#include <string>

namespace Lex {
	class Token { // TODO: turn this into a template class and replace individual token types with a single template one.
	public:
		friend class Lexer;
		friend class Keywords;
		Token();

		void SetType(TokenType token_type)
		{
			this->type_ = token_type;
		}

		TokenType Type() const
		{
			return type_;
		}

		std::string ToString() 
		{
			std::string str = std::to_string((int)this->type_);
			return str;
		}

		const std::string& Ident() const
		{
			return str_;
		}

		void SetIdent(const std::string& ident)
		{
			str_ = std::move(ident);
		}

		template<typename TokenElavType>
		void SetTokenType(TokenElavType type);

		template<typename TokenElavType>
		TokenElavType GetTokenType();

		template<typename TokenElavType>
		bool IsTokenType(TokenElavType type);

		TokenKeyword GetKeywordType()
		{
			return keyword_type;
		}
	private:
		TokenType type_;
		TokenPunctuator punctuator_type;
		TokenOperator operator_type;
		TokenKeyword keyword_type;
		TokenLiteral literal_type;
		std::string str_;
		int numeric_value; // Support other numeric types
	};
}

#endif