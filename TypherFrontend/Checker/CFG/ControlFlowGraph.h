#include "ASTNode.h"

#include "Expressions/Operator.h"
#include "Expressions/CallExpression.h"
#include "Expressions/MemoryOperation.h"
#include "Expressions/InitializerList.h"
#include "statements/ReturnStatement.h"
#include "statements/IfStatement.h"
#include "statements/WhileStatement.h"
#include "statements/ForStatement.h"
#include "Literals/IntegerLiteral.h"
#include "Literals/StringLiteral.h"
#include "statements/ExpressionStatement.h"

namespace Checker{
    enum class BasicBlockFlags : uint8_t {
        NONE = 0,
        ENTRY = 1U << 0U,
        EXIT = 1U << 1U,
        CONDITION = 1U << 2U,
        LOOP = 1U << 3U
    };

    class BasicBlock {
    public:
        friend class CFG;
        
        std::pair<size_t, size_t> AddSuccessor(BasicBlock *successor);
        std::pair<size_t, size_t> AddPredecessor(BasicBlock *predecessor);

        size_t GetIndex() const;
        BasicBlockFlags GetFlags() const;

        bool HasFlag(BasicBlockFlags flag) const;
        void SetFlag(BasicBlockFlags flag);
        void ClearFlag(BasicBlockFlags flag);

        const ArenaVector<BasicBlock *> &GetSuccessors() const;
        const ArenaVector<BasicBlock *> &GetPredecessors() const;
        const ArenaVector<AST::ASTNode *> &GetNodes() const;

        AST::ASTNode *GetLastNode() const;
        size_t GetSize() const;
        
        BasicBlock(ArenaAllocator *allocator, size_t index) noexcept;

    private:
        size_t AddNode(ir::AstNode *node);
        size_t index_;
        ArenaVector<ir::AstNode *> nodes_;
        ArenaVector<BasicBlock *> succs_;
        ArenaVector<BasicBlock *> preds_;
        BasicBlockFlags flags_ {BasicBlockFlags::NONE};
    };
    

    class ControlFlowGraph {
    public:
        explicit ControlFlowGraph(MemoryAllocator *allocator);

        BasicBlock *BuildExpressions(ir::AstNode *node, CFG::BasicBlock *bb);
        BasicBlock *BuildETSExpressions(ir::AstNode *node, CFG::BasicBlock *bb);
        BasicBlock *BuildStatements(ir::AstNode *node, CFG::BasicBlock *bb);

        BasicBlock* CreateBlock(AST::Function* node) override;
		BasicBlock* CreateBlock(AST::Statement* node) override;
		BasicBlock* CreateBlock(AST::VariableDeclarator* node) override;
        BasicBlock* CreateBlock(AST::Expression* node) override;
		BasicBlock* CreateBlock(AST::Identifier* node) override;
		BasicBlock* CreateBlock(AST::IntegerLiteral* node) override;
		BasicBlock* CreateBlock(AST::StringLiteral* node) override;
		BasicBlock* CreateBlock(AST::VariableDeclaration* node) override;
		BasicBlock* CreateBlock(AST::Operator* node) override;
		BasicBlock* CreateBlock(AST::CallExpression* node) override;
		BasicBlock* CreateBlock(AST::MemoryOperation* node) override;
		BasicBlock* CreateBlock(AST::ReturnStatement* node) override;
		BasicBlock* CreateBlock(AST::IfStatement* node) override;
		BasicBlock* CreateBlock(AST::ExpressionStatement* node) override;
		BasicBlock* CreateBlock(AST::WhileStatement* node) override;
		BasicBlock* CreateBlock(AST::ForStatement* node) override;
		BasicBlock* CreateBlock(AST::InitializerList* node) override;

    private:
        size_t basicBlockIdx_ {0};
        ArenaAllocator *allocator_;
        ArenaSet<BasicBlock *> blocks_;
    };
}