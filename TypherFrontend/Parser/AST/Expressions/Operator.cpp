#include "AST/Expressions/Operator.h"

namespace AST {
	Operator::Operator(Lex::TokenOperator op)
	{
		operator_ = op;
	}

	void Operator::SetLHS(AST::ASTNode* lhs)
	{
		lhs_ = lhs;
	}

	void Operator::SetRHS(AST::ASTNode* rhs)
	{
		rhs_ = rhs;
	}
}
