#include "ExpressionStatement.h"

namespace AST {
    ExpressionStatement::ExpressionStatement(Expression* expr)
    {
        expression_ = expr;
        expr->SetParent(this);
    }
}