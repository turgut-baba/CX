#ifndef LEXER_H
#define LEXER_H

#include "Tokens/TokenTypes.h"
#include <iostream>
#include "Tokens/Token.h"
#include "Keywords.h"
#include "Letters.h"

namespace Lexer {

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
		if (std::distance(iterator_, file_buffer.end()) >= fwd)
		{
			this->iterator_ += fwd;
		}
	}

	inline void IterBack(size_t bck = 1)
	{
		if (std::distance(file_buffer.begin(), iterator_) >= bck)
		{
			this->iterator_ -= bck;
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
	void ScanEqualSign();

	std::string file_buffer;
	std::string::iterator iterator_;
	size_t line_;
	Token current_token;
};

}

#endif