#ifndef VARIABLE_DECLARATOR_H
#define VARIABLE_DECLARATOR_H

#include "AST/Statement.h"
#include "AST/Expression.h"

namespace AST {
	class VariableDeclarator : public Statement {
	public:
		VariableDeclarator(Expression* exprs);

		virtual ~VariableDeclarator() = default;
/*
	private:
		Identifier target_ident;
		Expression* sub_expressions;
*/
	};
}

#endif