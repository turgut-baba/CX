#ifndef DECLARATOR_PARSER_H
#define DECLARATOR_PARSER_H

#include "Parser.h"
#include "ParserState.h"
#include "AST/Expressions/VariableDeclarator.h"

namespace Parser {
    class DeclaratorParser: public Parser{
    public:
		DeclaratorParser(std::shared_ptr<ParserState> state,
			DiagnosticEngine& diags, MemoryAllocator *allocator)
			: Parser(diags, allocator)
		{
			state_ = state;
		}
    private:
        AST::VariableDeclarator* parse_declarator();
    };
} // namespace Parser

#endif