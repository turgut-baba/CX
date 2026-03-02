#include "AST/Expressions/Operator.h"

namespace AST {
	Operator::Operator(Lex::TokenOperator op)
		: Expression(AstNodeType::OPERATOR)
	{
		operator_ = op;
		switch(operator_) {
			case Lex::TokenOperator::ADD:
				OpType = OperatorKind::ADD;
				break;
			case Lex::TokenOperator::EQUALS:
				OpType = OperatorKind::EQS;
				break;
			case Lex::TokenOperator::ASSIGNMENT:
				OpType = OperatorKind::ASN;
				break;
			case Lex::TokenOperator::LESS_OR_EQUAL:
				OpType = OperatorKind::LEQ;
				break;
			case Lex::TokenOperator::LESSER:
				OpType = OperatorKind::LES;
				break;
			case Lex::TokenOperator::GREAT_OR_EQUAL:
				OpType = OperatorKind::GEQ;
				break;
			case Lex::TokenOperator::GREATER:
				OpType = OperatorKind::GRT;
				break;
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
