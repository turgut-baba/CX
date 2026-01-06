#ifndef OPERATOR_H
#define OPERATOR_H

#include "AST/ASTNode.h"

namespace AST {
	class Operator : public ASTNode {
	public:
		Operator() = default;

		virtual ~Operator() = default;
	};
}

#endif