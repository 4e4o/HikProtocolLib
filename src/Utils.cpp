#include "Utils.hpp"

#include <boost/beast/core/detail/base64.hpp>

#include <openssl/sha.h>

int Utils::roundUp(int numToRound, int multiple) {
    return ((numToRound + multiple - 1) / multiple) * multiple;
}

std::string Utils::sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    std::string result;
    char hex_array[3];

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.data(), input.size());
    SHA256_Final(hash, &sha256);

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_array, "%02x", hash[i]);
        hex_array[2] = 0;
        result += hex_array;
    }

    return result;
}

std::string Utils::base64Decode(const std::string& input) {
    std::vector<char> output(input.size());
    auto r = boost::beast::detail::base64::decode(output.data(), input.data(), input.size());
    output.resize(r.first);
    std::string result(output.begin(), output.end());
    return result;
}
