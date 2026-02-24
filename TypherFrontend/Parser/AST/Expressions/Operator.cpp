#include "AST/Expressions/Operator.h"

namespace AST {
	Operator::Operator(Lex::TokenOperator op)
		: Expression(AstNodeType::OPERATOR)
	{
		operator_ = op;
		switch(operator_) {
			case Lex::TokenOperator::ADD:
			{
				OpType = OperatorKind::ADD;
				break;
			}
			case Lex::TokenOperator::EQUALS:
			{
				std::cout << "in eq " << std::endl;
				OpType = OperatorKind::EQS;
				break;
			}
		} 
	}

	void Operator::SetLHS(AST::ASTNode* lhs)
	{
		lhs_ = lhs;
	}

	void Operator::SetRHS(AST::ASTNode* rhs)
	{
		rhs_ = rhs;
	}

	AST::ASTNode* Operator::GetLHS() 
	{
		return lhs_;
	}

	AST::ASTNode* Operator::GetRHS()
	{
		return rhs_;
	}
}
