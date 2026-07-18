#ifndef BODY_H
#define BODY_H

#include "ASTNode.h"
#include "Log/Debug.h"

namespace AST {

    class Body: public ASTNode {
    public:
        Body(SlabAllocator* alloc);

        Body(SlabVector<Statement*> statements);

        void SetOwner(Statement* owner);

        void Accept(NodeVisitor* v) override
        { 
            UNREACHABLE("Should not traverse body directly.");
        }

        void AddStatement(Statement* statement)
        {
            statements_.push_back(statement);
        }

        SlabVector<Statement*> Statements()
        {
            return statements_;
        }
    private:
        // Scope scope_;
        Statement* owner_;
        SlabVector<Statement*> statements_;
    };

}
#endif