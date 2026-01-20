#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include "Memory/MemAlloc.h"

namespace Parser {

	class StatementParser;
	class ExpressionParser;
	class AssignmentParser;

	struct ParserState {
		// StatementParser* statement_parser;
		std::unique_ptr<Lex::Lexer> lexer_;
		AST::Statement* AST_tree; // TODO: write a proper allocator instead of raw ptr for array.

		StatementParser* statement_parser;
		ExpressionParser* expression_parser;
		AssignmentParser* assignment_parser;

		MemoryAllocator allocator;
	};
}

#endif