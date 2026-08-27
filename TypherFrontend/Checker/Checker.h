#ifndef CHECKER_H
#define CHECKER_H

#include "Parser.h"
//#include "SemanticAnalyzer.h"
#include "Memory/MemAlloc.h"
#include "Memory/BumpPtrAlloc.h"

namespace checker {

	class Type;

	class Checker {
	public:
		explicit Checker(DiagnosticEngine &diags, MemoryAllocator *allocator);

        virtual void Check(Function* node) = 0;
        virtual void Check(Statement* node) = 0;
        virtual void Check(VariableDeclarator* node) = 0;
        virtual void Check(VariableDeclaration* node) = 0;
        virtual void Check(Expression* node) = 0;
        virtual void Check(Identifier* node) = 0;
        virtual void Check(IntegerLiteral* node) = 0;
        virtual void Check(StringLiteral* node) = 0;
        virtual void Check(Operator* node) = 0;
        virtual void Check(CallExpression* node) = 0;
        virtual void Check(MemoryOperation* node) = 0;
        virtual void Check(ReturnStatement* node) = 0;
        virtual void Check(IfStatement* node) = 0;
        virtual void Check(ExpressionStatement* node) = 0;
        virtual void Check(WhileStatement* node) = 0;
        virtual void Check(ForStatement* node) = 0;
        virtual void Check(InitializerList* node) = 0;

		void StartChecker(SlabVector<AST::Statement*>& AST_tree);

		template <AllocatorType Type = AllocatorType::SLAB>
		auto Allocator() -> typename AllocTypeMap<Type>::type*
		{
			if constexpr (Type == AllocatorType::DTOR)
				return allocator_->dtorAlloc.get();
			if constexpr (Type == AllocatorType::SLAB)
				return allocator_->slabAlloc.get();
			if constexpr (Type == AllocatorType::BUMP)
				return allocator_->bumpAlloc.get();
		}

		SemanticAnalyzer* GetAnalyzer() const
		{
			return state_->analyzer_.get();
		}

		SymbolTable* SymbolTable()
		{
			return state_->symbol_table.get();
		}

		void check_module(SlabVector<AST::Statement*> ASTTree);
	private:
		void CheckNode(AST::ASTNode* node);

		DiagnosticEngine &diags_;
		MemoryAllocator* allocator_;

		CheckerState* state_;
	};
}

#endif