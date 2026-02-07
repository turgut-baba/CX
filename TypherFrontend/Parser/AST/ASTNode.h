#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <vector>
#include "Memory/MemAlloc.h"
#include <functional>
#include "AST/Visitor.h"

namespace AST {

	enum NodeType { // TODO: move this to a seperate file
		NODE = 0,
		FUNCTION,
	};

	class ASTNode;

	using NodeFunction = std::function<ASTNode* (ASTNode*)>;
	using ConditionalNodeFunction = std::function<bool (ASTNode*)>;

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

		virtual std::string String()
		{
			return "Default node ";
		}

		void ChangeChildrenNodesRecursively(const NodeFunction& func);

		ASTNode* HasChild(const ConditionalNodeFunction& cb) const;
		virtual void Accept(NodeVisitor* visitor) = 0;

		bool IsNodeType(NodeType type)
		{
			return type == node_type_;
		}
	protected:
		std::vector<ASTNode*> children_; // TODO: convert to custom allocator
		ASTNode* parent_;
		NodeType node_type_;
		//Flags flags_{};
		// Scope* current_scope
	};
}

#endif