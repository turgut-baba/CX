#ifndef IF_STATEMENT_H
#define IF_STATEMENT_H

#include "ASTNode.h"
#include "Expression.h"

class IfStatement: public ASTNode {
public:
    IfStatement(Expression* expr)
    {
        // TODO: check if the expr is compatible with the if statement
    }

    
private:
    Expression* expr_;
    SlabVector<Statement*> body_;
};

#endif