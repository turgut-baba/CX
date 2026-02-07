# CMakeList.txt : CMake project for TypherCompiler, include source and define
# project specific logic here.
#

# Add source to this project's executable.


# MLIR SETUP --------------------

set(LLVM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/llvm-project" CACHE INTERNAL "")

set(LIBS
    MLIRAnalysis
    MLIRFunctionInterfaces
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
    MLIRSupport              
    LLVMSupport    
    MLIRControlFlowInterfaces # ADD THIS
    MLIRCallInterfaces        # ADD THIS
    MLIRCastInterfaces        # ADD THIS            
)

set(LLVM_BUILD_DIR "${LLVM_ROOT}/build") 

# 2. Point specifically to the folders containing the Config.cmake files
set(MLIR_DIR "${LLVM_BUILD_DIR}/lib/cmake/mlir")
set(LLVM_DIR "${LLVM_BUILD_DIR}/lib/cmake/llvm")

list(APPEND CMAKE_MODULE_PATH "${LLVM_PROJ_SRC}/llvm/cmake/modules")

# 3. Now find_package will work
find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH ${MLIR_CMAKE_DIR})
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})

include(AddLLVM)
include(TableGen)
include(AddMLIR)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${MLIR_GENERATED_INCLUDE_DIRS})

add_subdirectory(MLIR/Dialect)

set(LLVM_LINK_COMPONENTS
  Support
)

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
"Parser/AST/Identifier.cpp" 
"Parser/AST/statements/VariableDeclaration.cpp"
"Parser/AST/statements/VariableDeclarator.cpp"
"Checker/Checker.cpp" 
"Parser/AST/Expressions/CallExpression.cpp"
"Parser/AST/Expressions/Operator.cpp" 
"Parser/AST/Literals/Literal.h"
"MLIR/MLIRGen.cpp"
"MLIR/MLIRBuilder.cpp"
 )

target_include_directories(TypherCompiler
  PRIVATE
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Lexer
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Parser
  ${CMAKE_SOURCE_DIR}/TypherFrontend/Core
  ${CMAKE_SOURCE_DIR}/TypherFrontend/MLIR
)

target_include_directories(TypherCompiler SYSTEM PRIVATE 
    ${LLVM_INCLUDE_DIRS} 
    ${MLIR_INCLUDE_DIRS}
)
# DIALECT -------

# add_mlir_dialect(TypherOps Typher)



add_dependencies(TypherCompiler TypherDialect)

# MLIR SETU END --------------------

mlir_target_link_libraries(TypherCompiler
  PUBLIC
  TypherDialect
   ${LIBS}
  # ... other MLIR dependencies
)

llvm_config(TypherCompiler support core irreader)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TypherCompiler PROPERTY CXX_STANDARD 20)
endif()

if(LINUX)
  set(CMAKE_C_COMPILER clang)
  target_compile_options(TypherCompiler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# TODO: Add tests and install targets if needed.
