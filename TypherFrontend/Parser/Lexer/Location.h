#ifndef LOCATION_H
#define LOCATION_H

#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <map>

class Location {
public:
    Location() {}; // debug
    explicit Location(std::string file, unsigned int line, unsigned int col)
        :file(file), line(line), col(col) {}

    std::string file = "sanity.ty"; // debug
    unsigned int line = 1; // debug
    unsigned int col = 1; // debug

    Location* ParentLocation() const
    {
        return parentloc;
    }
private:
    Location* parentloc = nullptr;
};

#endif