#include "ASTNode.h"

namespace AST {
	void ASTNode::ChangeChildrenNodesRecursively(const NodeFunction& func)
	{
		auto ret = func(this);
		for (auto& child : this->Chlidren())
		{
			if (child != nullptr) child->ChangeChildrenNodesRecursively(func);
		}
	}

	void ASTNode::IterateTree(SlabVector<AST::Statement*>& AST_tree, std::function<void(AST::Statement*)> callback)
	{
		for (auto& node : AST_tree)
		{
			if (node != nullptr) {
				callback(node);
			}
		}
	}
}