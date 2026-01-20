#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <vector>

namespace AST {
	class ASTNode {
	public:
		virtual ~ASTNode() = default;

		std::vector<ASTNode*> Chlidren() 
		{
			return children_;
		}

		ASTNode* Parent()
		{
			return parent_;
		}

		void SetParent(ASTNode* parent)
		{
			parent_ = parent;
			parent->AddChildren(this);
		}

		void AddChildren(ASTNode* child)
		{
			children_.push_back(child);
		}
	private:
		std::vector<ASTNode*> children_; // TODO: convert to custom allocator
		ASTNode* parent_;
		//Flags flags_{};
		// Scope* current_scope
	};
}

#endif