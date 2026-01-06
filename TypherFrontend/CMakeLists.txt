# CMakeList.txt : CMake project for TypherCompiler, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (TypherCompiler "Core/TypherCompiler.cpp" "Core/TypherCompiler.h" "Core/CompilerImpl.cpp" "Core/CompilerImpl.h" "Lexer/Keywords.h" "Lexer/Lexer.h" "Lexer/Lexer.cpp" "Lexer/Tokens/TokenTypes.h" "Lexer/Tokens/Token.h" "Lexer/Tokens/Token.cpp" "Parser/Parser.h" "Lexer/Letters.h" "Parser/Parser.cpp" "Lexer/Keywords.cpp")

target_include_directories(TypherCompiler
    PRIVATE
        ${CMAKE_SOURCE_DIR}/TypherFrontend/Lexer
        ${CMAKE_SOURCE_DIR}/TypherFrontend/Parser
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TypherCompiler PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
