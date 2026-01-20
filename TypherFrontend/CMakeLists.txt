# CMakeList.txt : CMake project for TypherCompiler, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (TypherCompiler "Core/TypherCompiler.cpp" 
"Core/CompilerImpl.cpp" 
"Lexer/Lexer.cpp" 
"Lexer/Tokens/Token.cpp" 
"Parser/StatementParser.cpp"
"Parser/Parser.cpp" 
"Lexer/Keywords.cpp" 
"Parser/AST/ASTNode.cpp"  
"Parser/ExpressionParser.cpp" 
"Parser/AST/Statements.cpp"
"Parser/AST/Expressions/Identifier.cpp" 
"Parser/AssignmentParser.cpp" 
"Parser/AST/statements/VariableDeclaration.cpp"
"Parser/AST/statements/VariableDeclarator.cpp"
 "Checker/Checker.cpp" "Parser/AST/Expressions/CallExpression.cpp" "Parser/AST/Expressions/Operator.cpp" "Core/Memory/BumPtrAlloc.h")


target_include_directories(TypherCompiler
    PRIVATE
        ${CMAKE_SOURCE_DIR}/TypherFrontend/Lexer
        ${CMAKE_SOURCE_DIR}/TypherFrontend/Parser
        ${CMAKE_SOURCE_DIR}/TypherFrontend/Core
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TypherCompiler PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
