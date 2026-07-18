#ifndef DIRECTIVES_H
#define DIRECTIVES_H

#include <iostream>

namespace PreProcessor {

enum Directives {
    NONE,
    INCLUDE,
    DEFINE,
    IFNDEF,
    ELSE,
    ENDIF
};

struct DirectiveInfo {
    size_t line_number;
    std::string_view directive_text;
};

class DirectiveProcessor{
public:
    void Process(std::string& file, int directive_start);
};

} // namespace PreProcessor

#endif