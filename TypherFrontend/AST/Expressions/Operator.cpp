#include "Expressions/Operator.h"

namespace AST {
	Operator::Operator(Lex::TokenOperator op)
		: Expression(AstNodeType::OPERATOR)
	{
		operator_ = op;
		switch(operator_) {
			case Lex::TokenOperator::ADD:
				OpType = OperatorKind::ADD;
				break;
			case Lex::TokenOperator::SUBTRACT:
				OpType = OperatorKind::SUB;
				break;
			case Lex::TokenOperator::MULTIPLY:
				OpType = OperatorKind::MUL;
				break;
			case Lex::TokenOperator::DIVIDE:
				OpType = OperatorKind::DIV;
				break;
			case Lex::TokenOperator::MODULUS:
				OpType = OperatorKind::MOD;
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

	void Operator::SetLHS(AST::Expression* lhs)
	{
		lhs_ = lhs;
	}

	void Operator::SetRHS(AST::Expression* rhs)
	{
		rhs_ = rhs;
	}

	AST::Expression* Operator::GetLHS() 
	{
		return lhs_;
	}

	AST::Expression* Operator::GetRHS()
	{
		return rhs_;
	}
}
