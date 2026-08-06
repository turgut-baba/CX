#ifndef ASTNODE_H
#define ASTNODE_H

#include <iostream>
#include <vector>
#include "Memory/MemAlloc.h"
#include <functional>
#include "Visitor.h"
#include "Location.h"


enum class AstNodeType { // TODO: move this to a seperate file
	NODE = 0,
	FUNCTION,
	IDENTIFIER,
	VARIABLE_DECLARATOR,
	CALL_EXPR,
	OPERATOR,
	LITERAL,
	MEMORY_OPERATION,
	INITIALIZER_LIST,
};

enum class AstBuiltinTypes { // TODO: move this to a seperate file
	NONE = 0,
	VOID,
	BOOL,
	CHAR,
	INT,
	FLOAT,
	DOUBLE
};

namespace AST {

	class ASTNode;

	using NodeFunction = std::function<ASTNode* (ASTNode*)>;
	using ConditionalNodeFunction = std::function<bool (ASTNode*)>;

	class ASTNode {
	public:
		ASTNode() = default;
		ASTNode(AstNodeType type)
		: node_type_(type) {}
		virtual ~ASTNode() = default;

		std::vector<ASTNode*> Chlidren() 
		{
			return children_;
		}

		ASTNode* Parent()
		{
			return parent_;
		}

		virtual void SetParent(ASTNode* parent)
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

		bool IsNodeType(AstNodeType type)
		{
			return type == node_type_;
		}

		AstNodeType NodeType() const
		{
			return node_type_;
		}

		void SetLocation(Location loc)
		{
			loc_ = loc;
		}

		Location Loc() const
		{
			return loc_;
		}
	protected:
		std::vector<ASTNode*> children_; // TODO: convert to custom allocator
		ASTNode* parent_;
		AstNodeType node_type_;
		Location loc_; // debug
		//Flags flags_{};
		// Scope* current_scope
	};
}

#endif