#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"

namespace Parser {

class Parser {
public:
		Parser() = default;

		Parser(std::string& file_buffer);

		Lexer::Lexer* Lexer() const
		{
			return lexer_.get();
		}

		void parse();
private:
	std::unique_ptr<Lexer::Lexer> lexer_;
};

}

#endif