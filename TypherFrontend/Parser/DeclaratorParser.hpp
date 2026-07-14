#ifndef DECLARATOR_PARSER_H
#define DECLARATOR_PARSER_H

#include "Parser.h"
#include "ParserState.h"
#include "AST/Expressions/VariableDeclarator.h"
#include "AST/Statement.h"
#include "Helpers.h"

namespace Parser {
    class DeclaratorParser: public Parser{
    public:
		DeclaratorParser(std::shared_ptr<ParserState> state,
			DiagnosticEngine& diags, MemoryAllocator *allocator)
			: Parser(diags, allocator)
		{
			state_ = state;
		}

        AST::VariableDeclarator* parse_declarator();
        AST::Statement* VariableOrFunctionDecl();
    private:
        AST::Statement* ParseFunctionDeclaration(AST::VariableDeclarator* ident, AstBuiltinTypes return_type);
    };
} // namespace Parser

#endif