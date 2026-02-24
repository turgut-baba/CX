#include "AST/Expressions/VariableDeclarator.h"

namespace AST{
	VariableDeclarator::VariableDeclarator(Expression* expr, Identifier* ident)
		:ident_(ident), expression(expr), Expression(AstNodeType::VARIABLE_DECLARATOR)
	{
		if (expr != nullptr) {
			expr->SetParent(this);
		}
	}

	VariableDeclarator::VariableDeclarator(Identifier* ident)
		: ident_(ident), Expression(AstNodeType::VARIABLE_DECLARATOR)
	{
		if (ident != nullptr) {
			ident->SetParent(this);
		}
	}

}