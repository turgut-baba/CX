#ifndef PRE_PROCESSOR_H
#define PRE_PROCESSOR_H

#include <iostream>
#include "Directives.h"

namespace PreProcessor {

class PreProcessor{
public:
    PreProcessor(std::string& file_buffer);

    std::string Process();
private:
    std::string file_buffer_;
    DirectiveProcessor directives_;
};

} // namespace PreProcessor

#endif