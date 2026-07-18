#ifndef LEXER_H
#define LEXER_H

#include "Tokens/TokenTypes.h"
#include <iostream>
#include "Tokens/Token.h"
#include "Keywords.h"
#include "Letters.h"

namespace Lex {

class Lexer {
public:
	Lexer(const Lexer&) = delete;
	Lexer& operator=(const Lexer&) = delete;

	Lexer(std::string& file_buffer);

	size_t Line() const
	{
		return line_;
	};

	inline std::string::iterator& Iterator()
	{
		return iterator_;
	}

	inline void IterForward(size_t fwd = 1)
	{
		auto dist = std::distance(iterator_, file_buffer.end());

		if (dist > 0 && static_cast<size_t>(dist) >= fwd)
		{
			this->iterator_ += fwd;
			column_ += fwd;
		}
	}

	inline void IterBack(size_t bck = 1)
	{
		auto dist = std::distance(file_buffer.begin(), iterator_);

		if (dist > 0 && static_cast<size_t>(dist) >= bck)
		{
			this->iterator_ -= bck;
			column_ -= bck;
		}
	}

	Token& GetToken()
	{
		return current_token;
	}

	void NextToken();

	LexicalChar Peek()
	{
		return static_cast<LexicalChar>((iterator_ != file_buffer.end()) ? *iterator_ : '\0');
	}

private:
	template<typename TokenElem>
	void SetSpecificToken(TokenElem type) {
			current_token.SetType(TokenType::Punctuator);
			current_token.SetTokenType<TokenElem>(type);
	}
	void SkipWhiteSpaces();
	void ScanForEquality(std::string ident, TokenOperator single, TokenOperator equal);

	std::string file_buffer;
	std::string file_name;
	std::string::iterator iterator_;
	size_t line_ = 1; // We start at the first line.
	size_t column_ = 0; // We start by doing an IterForward so we start at one.
	Token current_token;
};

}

#endif