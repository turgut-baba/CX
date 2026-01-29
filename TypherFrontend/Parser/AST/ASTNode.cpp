#include "AST/ASTNode.h"

namespace AST {
	void ASTNode::ChangeChildrenNodesRecursively(const NodeFunction& func)
	{
		auto ret = func(this);
		for (auto& child : this->Chlidren())
		{
			if (child != nullptr) child->ChangeChildrenNodesRecursively(func);
		}
	}
}