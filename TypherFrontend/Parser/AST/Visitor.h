#ifndef VISITOR_H
#define VISITOR_H

namespace AST {
    class Function;
    class Statement;
    class VariableDeclarator;
    class VariableDeclaration;
    class Expression;
    class Identifier;
    class IntegerLiteral;
    class Operator;
    
    class NodeVisitor {
    public:
        virtual void Visit(Function* node) = 0;
        virtual void Visit(Statement* node) = 0;
        virtual void Visit(VariableDeclarator* node) = 0;
        virtual void Visit(VariableDeclaration* node) = 0;
        virtual void Visit(Expression* node) = 0;
        virtual void Visit(Identifier* node) = 0;
        virtual void Visit(IntegerLiteral* node) = 0;
        virtual void Visit(Operator* node) = 0;
    };
}

#endif