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
		}
	private:
		std::vector<ASTNode*> children_; // TODO: convert to custom allocator
		ASTNode* parent_;
		//Flags flags_{};
		// Scope* current_scope
	};
}

#endif