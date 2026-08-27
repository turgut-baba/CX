#include "ASTNode.h"

namespace Checker {
    enum TypeKind {
        UNDEFINED,
        BUILT_IN,
        USER_DEFINED,
    };

    enum Flags {
        CONST = 1 << 0,
        VOLATILE = 1 << 1,
        POINTER = 1 << 2,
        ARRAY = 1 << 3,
        FUNCTION = 1 << 4
    };
    
    class Type {
    public:
        Type() = delete;
        Type(AstBuiltinTypes kind); // For BUILT_IN
        Type(std::string name, int size); // For USER_DEFINED

        TypeKind Kind() const 
        { 
            return kind_;
        }

        bool IsSameAs(const Type& other) const 
        {
            if(kind_ == TypeKind::USER_DEFINED && other.kind_ == TypeKind::USER_DEFINED) {
                return name_ == other.name_;
            }

            return kind_ == other.kind_ && size_in_bytes == other.size_in_bytes;
        }

        int Flags() const { return flags_; }

        void SetFlag(Flags flag) { flags_ |= flag; }
        void ClearFlag(Flags flag) { flags_ &= ~flag; }
        bool HasFlag(Flags flag) const { return (flags_ & flag) != 0; }

        static std::shared_ptr<Type> MakeVoid() { return std::make_shared<Type>(TypeKind::VOID, 0); }
        static std::shared_ptr<Type> MakeByte() { return std::make_shared<Type>(TypeKind::BYTE, 1); }
        static std::shared_ptr<Type> MakeChar() { return std::make_shared<Type>(TypeKind::CHAR, 1); }
        static std::shared_ptr<Type> MakeShort() { return std::make_shared<Type>(TypeKind::SHORT, 2); }
        static std::shared_ptr<Type> MakeInt() { return std::make_shared<Type>(TypeKind::INT, 4); }
        static std::shared_ptr<Type> MakeLong() { return std::make_shared<Type>(TypeKind::LONG, 8); }
        static std::shared_ptr<Type> MakeFloat() { return std::make_shared<Type>(TypeKind::FLOAT, 4); }
        static std::shared_ptr<Type> MakeDouble() { return std::make_shared<Type>(TypeKind::DOUBLE, 8); }
    private:
        int size_in_bytes;
        std::string name_ = "";  // For USER_DEFINED
        AstBuiltinTypes builtin_ = AstBuiltinTypes::NONE; // For BUILT_IN
        SlabArena<std::shared_ptr<Type>> param_types;  // For FUNCTION
        TypeKind kind_;
        int flags_;
    };
}