#include "Parser.h"
#include "Tokens/TokenTypes.h"

namespace Parser {
	Parser::Parser(std::string& file_buffer) {
		this->lexer_ = std::make_unique<Lexer::Lexer>(file_buffer);
	}

	void Parser::parse() 
	{
		while (Lexer()->GetToken().Type() != Lexer::TokenType::EOS) {
			Lexer()->NextToken();
			std::cout << "Token: " << Lexer()->GetToken().Ident() << std::endl;
		}
	}
}