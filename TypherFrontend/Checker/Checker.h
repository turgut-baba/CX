#ifndef CHECKER_H
#define CHECKER_H

#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "Memory/MemAlloc.h"
#include "Memory/BumpPtrAlloc.h"

namespace checker {

	class Type;

	class Checker {
	public:
		explicit Checker(DiagnosticEngine &diags, MemoryAllocator *allocator);

		//NO_COPY_SEMANTIC(Checker);
    	// NO_MOVE_SEMANTIC(Checker);

		Type *BuiltinByteType() const;
		Type *BuiltinShortType() const;
		Type *BuiltinIntType() const;
		Type *BuiltinLongType() const;
		Type *BuiltinFloatType() const;
		Type *BuiltinDoubleType() const;
		Type *BuiltinCharType() const;

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
			return analyzer_.get();
		}

		void check_module(SlabVector<AST::Statement*> ASTTree);
	private:
		void CheckNode(AST::ASTNode* node);

		std::unique_ptr<SemanticAnalyzer> analyzer_;
		DiagnosticEngine &diags_;
		MemoryAllocator* allocator_;
		std::unique_ptr<SlabVector<AST::Statement*>> AST_tree;
	};
}

#endif