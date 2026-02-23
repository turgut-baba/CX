#ifndef LITERAL_H
#define LITERAL_H

#include "AST/ASTNode.h"

namespace AST {
	class Literal: public Expression {
	public:
		Literal() : Expression(AstNodeType::LITERAL) { }
	};
}

#endif