#include "VariableDeclarator.h"

namespace AST{
	VariableDeclarator::VariableDeclarator(Expression* expr, Identifier* ident)
		:ident_(ident), expression(expr)
	{
		if (expr != nullptr) {
			expr->SetParent(this);
		}
	}

	VariableDeclarator::VariableDeclarator(Identifier* ident)
		: ident_(ident)
	{
		if (ident != nullptr) {
			ident->SetParent(this);
		}
	}

}