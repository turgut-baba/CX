#ifndef STATEMENT_PARSER_H
#define STATEMENT_PARSER_H

#include "Parser.h"
#include "AST/Statement.h"
#include "ParserState.h"
#include "AST/Function.h"
#include "AST/statements/ReturnStatement.h"
#include "AST/statements/IfStatement.h"
#include "AST/statements/WhileStatement.h"
#include "AST/statements/ExpressionStatement.h"

namespace Parser {
	class StatementParser : public Parser 
	{
	public:
		StatementParser(std::shared_ptr<ParserState> state, 
			DiagnosticEngine& diags, MemoryAllocator *allocator);

		AST::Statement* parse_statement();

		AST::Body* ParseBody(AST::Statement* body);

	private:
		AST::Statement* HandleKeywords();
		AST::Statement* HandleIdentifier();

		AST::IfStatement* HandleIfKeyword();
		AST::WhileStatement* HandleWhileKeyword();

		AST::ReturnStatement* HandleReturnStatement();
	};
}

#endif