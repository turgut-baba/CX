#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "Expression.h"
#include "Expressions/Operator.h"
#include "Literals/IntegerLiteral.h"

namespace checker {
    class SemanticAnalyzer {
    public:
        void Check(AST::ASTNode *node);
        /*checker::Type*/ void Check(AST::IntegerLiteral *node);
    };
}

#endif