#ifndef ASSIGNMENT_PARSER_H
#define ASSIGNMENT_PARSER_H

#include "Parser.h"
#include "ParserState.h"
#include "AST/statements/VariableDeclaration.h"
#include "AST/statements/VariableDeclarator.h"

namespace Parser {
	class AssignmentParser : public Parser {
	public:
		AssignmentParser(std::shared_ptr<ParserState> state);
		
		AST::Statement* parse_assignment();

		AST::VariableDeclarator* ParseDeclarator();
	};
};

#endif