#ifndef VARIABLE_DECLARATOR_H
#define VARIABLE_DECLARATOR_H

#include "AST/Statement.h"
#include "AST/Expression.h"
#include "AST/Identifier.h"
#include "AST/Visitor.h"

namespace AST {
	class VariableDeclarator : public Expression {
	public:
		VariableDeclarator()
			: Expression(AstNodeType::VARIABLE_DECLARATOR) {}

		VariableDeclarator(Expression* expr, Identifier* ident);

		VariableDeclarator(Identifier* ident);

		virtual ~VariableDeclarator() = default;

		std::string Name()
		{
			return ident_->Value();
		}
		
		void Accept(NodeVisitor* v) override { v->Visit(this); }

		Expression* Expr() const
		{
			return expression;
		}

		std::string String() override
		{
			return "DECLARATOR node";
		}

	private:
		Identifier* ident_;
		Lex::TokenKeyword type_; // TODO: turn this into a type class.
		Expression* expression;
	};
}

#endif