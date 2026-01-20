#ifndef STATEMENT_PARSER_H
#define STATEMENT_PARSER_H

#include "Parser.h"
#include "AST/Statement.h"
#include "ParserState.h"

namespace Parser {
	class StatementParser : public Parser 
	{
	public:
		StatementParser(std::shared_ptr<ParserState> state);

		AST::Statement* parse_statement();
	private:
		AST::Statement* HandleKeywords();
		AST::Statement* PottentialVariableOrFunctionDecl();
	};
}

#endif