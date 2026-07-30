#ifndef COMMENTS_H
#define COMMENTS_H

#include <string>
#include <string_view>

namespace PreProcessor {

void strip_comments_in_place(std::string& source);

} // namespace PreProcessor

#endif