#include "AST/statements/ReturnStatement.h"

namespace AST {
    ReturnStatement::ReturnStatement(Expression* expr)
        : ReturnedExpression_(expr)
    {
        expr->SetParent(this);
    }


    ReturnStatement::ReturnStatement(Expression* expr,  Function* func)
            : ReturnedExpression_(expr) ,OwnerFunction_(func)
    {
        expr->SetParent(this);
    }
}