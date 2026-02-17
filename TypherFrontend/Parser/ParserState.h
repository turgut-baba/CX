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
		// StatementParser* statement_parser;
		std::unique_ptr<Lex::Lexer> lexer_;
		AST::Statement* AST_tree; // TODO: write a proper allocator instead of raw ptr for array.

		StatementParser* statement_parser;
		ExpressionParser* expression_parser;

		MemoryAllocator allocator {};

		DiagnosticEngine diags;

		ParserState() {
			allocator.dtorAlloc = std::make_shared<DtorMemAllocator>();
			allocator.slabAlloc = std::make_shared<SlabAllocator>(1024 * 1024);
			allocator.bumpAlloc = std::make_shared<BumpPtrAllocator>(1024 * 1024);
		}
	};
}

#endif