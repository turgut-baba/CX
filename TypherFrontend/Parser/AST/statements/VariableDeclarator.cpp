#include "VariableDeclarator.h"

namespace AST{
	VariableDeclarator::VariableDeclarator(Expression* expr, Identifier* ident)
		:ident_(ident)
	{
		if (expr != nullptr) {
			expr->SetParent(this);
		}
	}

}