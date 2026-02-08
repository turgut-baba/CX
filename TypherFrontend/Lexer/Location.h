#ifndef LOCATION_H
#define LOCATION_H

class Location {
public:
    Location() {}; // debug
    explicit Location(std::string file, unsigned int line, unsigned int col);

    std::string file = "sanity.ty"; // debug
    unsigned int line = 1; // debug
    unsigned int col = 1; // debug
};

#endif