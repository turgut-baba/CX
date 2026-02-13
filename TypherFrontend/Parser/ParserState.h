#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <memory>
#include "Memory/MemAlloc.h"

namespace Parser {

	class StatementParser;
	class ExpressionParser;

	struct ParserState {
		// StatementParser* statement_parser;
		std::unique_ptr<Lex::Lexer> lexer_;
		AST::Statement* AST_tree; // TODO: write a proper allocator instead of raw ptr for array.

		StatementParser* statement_parser;
		ExpressionParser* expression_parser;

		MemoryAllocator allocator {};

		ParserState() {
			allocator.dtorAlloc = std::make_unique<DtorMemAllocator>();
			allocator.slabAlloc = std::make_unique<SlabAllocator>(1024 * 1024);
			allocator.bumpAlloc = std::make_unique<BumpPtrAllocator>(1024 * 1024);
		}
	};
}

#endif