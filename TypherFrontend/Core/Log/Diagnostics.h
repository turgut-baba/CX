#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Location.h"
#include "Log/Debug.h"

enum class DiagLevel { Message, Warning, Error, Fatal, Success };

class DiagnosticEngine {
    int error_count = 0;
    int warning_count = 0;

public:
    // A small helper to wrap the stream-like building of an error
    struct DiagnosticBuilder {
        DiagnosticEngine& parent;
        DiagLevel level;
        Location loc;
        bool message = false;
        std::stringstream ss;

        ~DiagnosticBuilder()
        {
            if(!message) {
                parent.emit(*this);
            } else {
                parent.message(*this);
            }
            if (level == DiagLevel::Fatal)
                exit(1);
        }

        template <typename T>
        DiagnosticBuilder& operator<<(const T& msg) {
            ss << msg;
            return *this;
        }

        friend class DiagnosticEngine;
    };

    // Entry point. usage: diag.report(loc, DiagLevel::Error) << "Undefined variable " << varName;
    template<DiagLevel level>
    DiagnosticBuilder report(Location loc)
    {
        if constexpr (level == DiagLevel::Error || level == DiagLevel::Fatal) 
            error_count++;
        if constexpr (level == DiagLevel::Warning) 
            warning_count++;
        return {*this, level, loc};
    }

    template<DiagLevel level>
    DiagnosticBuilder report()
    {
        COMPILER_STATIC_ASSERT_MSG( 
            (level == DiagLevel::Message || level == DiagLevel::Success),
            "Need location info for diagnostic." );
        return {*this, level, {}, true};
    }


    void emitBacktrace(const Location* loc)
    {
        if (!loc || loc->ParentLocation() == nullptr) return;

        // Recursive call to print the oldest parent first
        emitBacktrace(loc->ParentLocation());

        // Print the breadcrumb
        std::cerr << "In file included from " << loc->ParentLocation()->file 
                << ":" << loc->ParentLocation()->line << ":\n";
    }

    void message(const DiagnosticBuilder& db)
    {
        printLevel(db.level);
        std::cerr << db.ss.str() << "\n";
    }

    void emit(const DiagnosticBuilder& db) {
        emitBacktrace(&db.loc);

        std::cerr << db.loc.file << ":" << db.loc.line << ":" << db.loc.col << ": ";
        
        message(db);

        printSourceLine(db.loc);
    }

    bool hasErrors() const { return error_count > 0; }

private:
    void printLevel(DiagLevel level) {
        switch (level) {
            case DiagLevel::Success: std::cerr << "\033[1;32msuccess: \033[0m"; break; 
            case DiagLevel::Error:   std::cerr << "\033[1;31merror: \033[0m"; break;
            case DiagLevel::Warning: std::cerr << "\033[1;33mwarning: \033[0m"; break;
            case DiagLevel::Message:    std::cerr << "\033[1;36mmessage: \033[0m"; break;
            case DiagLevel::Fatal:   std::cerr << "\033[1;41mfatal error: \033[0m"; break;
        }
    }

    void printSourceLine(Location loc) {
        // In a real compiler, you'd use a SourceManager to get the actual line text.
        // Mocking it here:
        std::string mockLine = "    int x = \"string\";"; 
        std::cerr << mockLine << "\n";
        
        // Print the caret (^)
        for (int i = 0; i < loc.col; ++i) std::cerr << " ";
        std::cerr << "\033[1;32m^\033[0m\n";
    }
};

#endif