#ifndef LOGGER_H
#define LOGGER_H

class Logger{
    Logger(std::string ch) : channel(ch) {
        // Only active if global debug is on AND (channel matches or no channel specified)
        active = DebugConfig::EnableDebug && 
                 (DebugConfig::CurrentChannel.empty() || DebugConfig::CurrentChannel == channel);
    }

    // Overload the << operator to behave like std::cout
    template <typename T>
    Logger& operator<<(const T& msg) {
        if (active) {
            std::clog << "[" << channel << "] " << msg;
        }
        return *this;
    }

    // Handle manipulators like std::endl
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (active) {
            manip(std::clog);
        }
        return *this;
    }
};

#endif