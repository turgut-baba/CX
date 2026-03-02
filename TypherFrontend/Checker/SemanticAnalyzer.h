#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "AST/Expression.h"
#include "AST/Expressions/Operator.h"
#include "AST/Literals/IntegerLiteral.h"

namespace checker {
    class SemanticAnalyzer {
    public:
        void Check(AST::ASTNode *node);
        /*checker::Type*/ void Check(AST::IntegerLiteral *node);
    };
}

#endif