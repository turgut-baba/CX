#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <memory>
#include "Memory/MemAlloc.h"
#include "Lexer.h"
#include "Log/Diagnostics.h"

namespace Parser {

	class StatementParser;
	class ExpressionParser;

	struct ParserState {
		std::unique_ptr<Lex::Lexer> lexer_;

		StatementParser* statement_parser;
		ExpressionParser* expression_parser;
	};
}

#endif