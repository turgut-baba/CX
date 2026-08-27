#include "Checker.h"

namespace checker {
    Checker::Checker(DiagnosticEngine &diags, MemoryAllocator *allocator)
        : diags_(diags), allocator_(allocator)
    {
        analyzer_ = std::make_unique<SemanticAnalyzer>();
        type_checker_ = std::make_unique<TypeChecker>();
    }

    void Checker::CheckNode(AST::ASTNode* node)
	{
		analyzer_->Check(node);

		for (auto& child : node->Chlidren()) {
			if (child != nullptr) CheckNode(child);
		}
	}

    void Checker::StartChecker(SlabVector<AST::Statement*>& AST_tree)
    {
        type_checker_->RegisterTypes(*state_->symbol_table_);
        
        analyzer_->Check(AST_tree, symbol_table_.get(), diags_, allocator_);

        
        for (auto& node : AST_tree)
		{
			if (node != nullptr) {
                for (auto& child : node->Chlidren())
                {
                    if (child != nullptr) CheckNode(child);
                }
				
			}
		}
    }

}