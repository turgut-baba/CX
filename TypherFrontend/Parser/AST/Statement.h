#ifndef STATEMENT_H
#define STATEMENT_H

#include "ASTNode.h"

namespace AST {
	class Statement : public ASTNode {
	protected:
		virtual ~Statement() = default;

		std::string String() override
		{
			return "Statement node ";
		}
	};
}

#endif