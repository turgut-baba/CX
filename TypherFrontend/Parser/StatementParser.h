#ifndef STATEMENT_PARSER_H
#define STATEMENT_PARSER_H

#include "Parser.h"
#include "Statement.h"
#include "ParserState.h"
#include "Function.h"
#include "statements/ReturnStatement.h"
#include "statements/IfStatement.h"
#include "statements/WhileStatement.h"
#include "statements/ForStatement.h"
#include "statements/ExpressionStatement.h"

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
		AST::ForStatement* HandleForKeyword();

		AST::ReturnStatement* HandleReturnStatement();
	};
}

#endif