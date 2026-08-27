#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "SymbolTable.h"
#include "Checker.h"

namespace Checker {

class TypeResolver: public Checker
{
public:
    TypeResolver();

    void RegisterTypes(SymbolTable& symbol_table);

    void CheckTypes(SymbolTable& symbol_table);
private:
    void CheckBuiltinType(const std::shared_ptr<Type>& type);
    void CheckUserDefinedType(const std::shared_ptr<Type>& type);
};

}

#endif // TYPE_CHECKER_H