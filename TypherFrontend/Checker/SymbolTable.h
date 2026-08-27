#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

#include "DiagnosticEngine.h"
#include "Memory/MemAlloc.h"

#include "Type.h"

namespace Checker {

enum class SymbolKind {
    TYPE_DEF,   // Represents a type definition (struct shell, typedef, primitive)
    VARIABLE,   // Represents a local/global variable binding
    FUNCTION    // Represents a function declaration or definition
};

class Symbol {
public:
    std::string name;
    SymbolKind kind;
    
    std::shared_ptr<Type> type;

    // Optional flags for semantic checking
    bool is_defined = true;
};

class SymbolTable {
public:
    SymbolTable()
    {
        scopes.push_back({});
    }
    
    void EnterScope()
    {
        scopes.push_back({});
    }

    void ExitScope()
    {
        TYPHER_ASSERT(scopes.size() > 1 && "Cannot exit top-level global scope!");
        scopes.pop_back();
    }

    bool IsGlobalScope() const 
    {
        return scopes.size() == 1;
    }

    Symbol* Lookup(const std::string& name)
    {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }
        return nullptr; 
    }

    std::shared_ptr<Type> LookupType(const std::string& name)
    {
        Symbol* sym = Lookup(name);
        if (sym && sym->kind == SymbolKind::TYPE_DEF) {
            return sym->type;
        }
        return nullptr;
    }

    Symbol* LookupName(const std::string& name)
    {
        Symbol* sym = Lookup(name);
        if (sym && (sym->kind == SymbolKind::VARIABLE || sym->kind == SymbolKind::FUNCTION)) {
            return sym;
        }
        return nullptr;
    }   
private:
    std::vector<std::unordered_map<std::string, Symbol>> stack;
};

}