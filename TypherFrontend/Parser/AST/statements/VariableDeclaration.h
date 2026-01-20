#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

#include "VariableDeclarator.h"

namespace AST {
	class VariableDeclaration : public Statement {
	public:
		VariableDeclaration(ArrayAlloc<VariableDeclarator*> decls);
		VariableDeclaration() = default;

		virtual ~VariableDeclaration() = default;
	private:
		VariableDeclarator* declarators_;
		bool is_definition = true;
	};
}

#endif