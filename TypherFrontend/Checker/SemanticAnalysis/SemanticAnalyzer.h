#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "Checker.h"
#include "Expression.h"
#include "Expressions/Operator.h"
#include "Literals/IntegerLiteral.h"

namespace Checker {
    class SemanticAnalyzer: public Checker {
    public:
        void BeginSemanticAnalyzer();
    private:
        void Check(AST::ASTNode *node);
        void Check(AST::IntegerLiteral *node);
        void Check(AST::VariableDeclaration *node);
        std::unique_ptr<SymbolTable> symbol_table_;
    };
}

#endif