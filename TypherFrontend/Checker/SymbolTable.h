#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

class SymbolTable {
    // A stack of scopes. Each scope is a map of Name -> Type
    std::vector<std::unordered_map<std::string, std::string>> stack;

public:
    void pushScope() { stack.push_back({}); }
    void popScope()  { stack.pop_back(); }

    // Declare a variable in the current (top) scope
    bool declare(const std::string& name, const std::string& type) {
        auto& currentScope = stack.back();
        if (currentScope.find(name) != currentScope.end()) {
            return false; // Error: Redefinition in same scope
        }
        currentScope[name] = type;
        return true;
    }

    // Lookup a variable by searching from the innermost scope outwards
    std::string lookup(const std::string& name) {
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            if (it->find(name) != it->end()) {
                return (*it)[name];
            }
        }
        return ""; // Error: Not found
    }
};