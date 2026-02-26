#ifndef STATEMENT_PARSER_H
#define STATEMENT_PARSER_H

#include "Parser.h"
#include "AST/Statement.h"
#include "ParserState.h"
#include "AST/Function.h"
#include "AST/statements/ReturnStatement.h"
#include "AST/statements/IfStatement.h"

namespace Parser {
	class StatementParser : public Parser 
	{
	public:
		StatementParser(std::shared_ptr<ParserState> state);

		AST::Statement* parse_statement();

		AST::Statement* ParseFunction(AST::Identifier* ident);
	private:
		SlabVector<AST::VariableDeclarator*> ParseDeclarators(AST::Identifier* ident);
		AST::Statement* HandleKeywords();
		AST::IfStatement* HandleIfKeyword();
		AST::Statement* PottentialVariableOrFunctionDecl();
		
		AST::ReturnStatement* HandleReturnStatement();
		AST::Body* ParseBody(AST::Statement* body);
	};
}

#endif