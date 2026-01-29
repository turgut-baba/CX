#ifndef VARIABLE_DECLARATION_H
#define VARIABLE_DECLARATION_H

#include "VariableDeclarator.h"

namespace AST {
	class VariableDeclaration : public Statement {
	public:
		VariableDeclaration(ArrayAlloc<VariableDeclarator*> decls);

		VariableDeclaration(VariableDeclarator* declarator) 
			: declarator_(declarator) 
		{
			declarator->SetParent(this);
		}

		VariableDeclaration() = default;

		virtual ~VariableDeclaration() = default;

		std::string String() override
		{
			return "DECLARATION node";
		}
	private:
		ArrayAlloc<VariableDeclarator*> declarators_;
		VariableDeclarator* declarator_;
		bool is_definition = true;
	};
}

#endif