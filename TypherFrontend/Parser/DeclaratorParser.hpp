#ifndef DECLARATOR_PARSER_H
#define DECLARATOR_PARSER_H

#include "Parser.h"
#include "ParserState.h"
#include "Expressions/VariableDeclarator.h"
#include "Statement.h"
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
        void ParseParameter(AST::Function* functionDecl);
        AST::Statement* ParseFunctionDeclaration(AST::VariableDeclarator* ident, AstBuiltinTypes return_type);
    };
} // namespace Parser

#endif