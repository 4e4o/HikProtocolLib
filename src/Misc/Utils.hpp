#ifndef UTILS_H
#define UTILS_H

#include <string>

class Utils {
public:
    static int roundUp(int numToRound, int multiple);
    static std::string sha256(const std::string& input);
    static std::string base64Decode(const std::string& input);
};

#endif /* UTILS_H */
