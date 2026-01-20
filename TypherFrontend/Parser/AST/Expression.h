#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ASTNode.h"

namespace AST {
	class Expression: public ASTNode {
	public:
		virtual ~Expression() = default;
	};
}

#endif