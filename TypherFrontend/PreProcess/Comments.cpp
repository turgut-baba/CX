#include "Comments.h"

namespace PreProcessor {

enum class LexState {
    Normal,
    InString,
    InSingleComment,
    InMultiComment
};

void strip_comments_in_place(std::string& source) {
    LexState state = LexState::Normal;
    bool escaped = false;
    const size_t len = source.length();

    for (size_t i = 0; i < len; ++i) {
        char c = source[i];

        switch (state) {
            case LexState::Normal: {
                if (c == '"') {
                    state = LexState::InString;
                    escaped = false;
                } 

                // Single-line comment: //
                else if (c == '/' && i + 1 < len && source[i + 1] == '/') {
                    state = LexState::InSingleComment;
                    source[i] = ' ';
                    source[i + 1] = ' ';
                    ++i; // Skip second slash
                }

                // Multi-line comment: /*
                else if (c == '/' && i + 1 < len && source[i + 1] == '*') {
                    state = LexState::InMultiComment;
                    source[i] = ' ';
                    source[i + 1] = ' ';
                    ++i; // Skip asterisk
                }
                break;
            }

            case LexState::InString: {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true; // Handle escaped characters (like \")
                } else if (c == '"') {
                    state = LexState::Normal;
                }
                break;
            }

            case LexState::InSingleComment: {
                if (c == '\n') {
                    state = LexState::Normal;
                } else {
                    source[i] = ' ';
                }
                break;
            }

            case LexState::InMultiComment: {
                // End of block comment: */
                if (c == '*' && i + 1 < len && source[i + 1] == '/') {
                    source[i] = ' ';
                    source[i + 1] = ' ';
                    ++i; // Skip trailing slash
                    state = LexState::Normal;
                } else if (c != '\n') {
                    // Blank out everything EXCEPT embedded newlines
                    source[i] = ' ';
                }
                break;
            }
        }
    }
}

} // namespace PreProcessor