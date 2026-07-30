#include "PreProcessor.h"
#include <vector>

namespace PreProcessor {

PreProcessor::PreProcessor(std::string& file_buffer)
{
    file_buffer_ = std::move(file_buffer);
}

std::string PreProcessor::Process()
{
    std::string processed_file;   

    size_t i = 0;
    size_t current_line = 1;
    size_t buffer_size = file_buffer_.size();

    while (i < buffer_size) {
        char c = file_buffer_[i];

        // Track line numbers for accurate compiler error reporting
        if (c == '\n') {
            current_line++;
            i++;
            continue;
        }

        // Skip Single-line Comments (//)
        if (c == '/' && (i + 1 < buffer_size) && file_buffer_[i + 1] == '/') {
            i += 2;
            while (i < buffer_size && file_buffer_[i] != '\n') {
                i++;
            }
            continue; 
        }

        // Skip Multi-line Comments (/* */)
        if (c == '/' && (i + 1 < buffer_size) && file_buffer_[i + 1] == '*') {
            i += 2;
            while (i + 1 < buffer_size && !(file_buffer_[i] == '*' && file_buffer_[i + 1] == '/')) {
                if (file_buffer_[i] == '\n') current_line++;
                i++;
            }
            i += 2; // skip past '*/'
            continue;
        }

        // Found the Preprocessor Directive symbol
        if (c == '#') {
            size_t start_of_directive = i;
            
            while (i < buffer_size && file_buffer_[i] != '\n') {
                if (file_buffer_[i] == '\\' && (i + 1 < buffer_size) && file_buffer_[i + 1] == '\n') {
                    current_line++;
                    i += 2;
                    continue;
                }
                i++;
            }

            std::string_view view(&file_buffer_[start_of_directive], i - start_of_directive);
            //directives_.Process(view, current_line);
            
            continue; 
        }

        i++;
    }

    return processed_file;
}

} // namespace PreProcessor