#ifndef STATEMENT_H
#define STATEMENT_H

#include "ASTNode.h"

namespace AST {
	class Statement : public ASTNode {
	protected:
		virtual ~Statement() = default;
	};
}

#endif