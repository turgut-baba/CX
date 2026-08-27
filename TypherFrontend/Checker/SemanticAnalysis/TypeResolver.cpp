#include "TypeResolver.h"

namespace Checker {
    TypeResolver::RegisterTypes(SymbolTable& symbol_table)
    {
        // Register built-in types
        symbol_table.declare("void", Symbol{"void", SymbolKind::TYPE_DEF, Type::MakeVoid()});
        symbol_table.declare("byte", Symbol{"byte", SymbolKind::TYPE_DEF, Type::MakeByte()});
        symbol_table.declare("char", Symbol{"char", SymbolKind::TYPE_DEF, Type::MakeChar()});
        symbol_table.declare("short", Symbol{"short", SymbolKind::TYPE_DEF, Type::MakeShort()});
        symbol_table.declare("int", Symbol{"int", SymbolKind::TYPE_DEF, Type::MakeInt()});
        symbol_table.declare("float", Symbol{"float", SymbolKind::TYPE_DEF, Type::MakeFloat()});
        symbol_table.declare("double", Symbol{"double", SymbolKind::TYPE_DEF, Type::MakeDouble()});

        AST::IterateTree(*state_->AST_tree, [&](AST::ASTNode* node) {
            // TODO: We will check user defined types here. For example, if we encounter a struct or class declaration, we can register it in the symbol table.
        });
    }

}