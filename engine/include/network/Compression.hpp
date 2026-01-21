#pragma once

#include <cstdint>
#include <vector>

namespace Network {

class Compression {
public:
    // Simple Run-Length Encoding (RLE)
    static std::vector<char> compress(const std::vector<char>& data) {
        std::vector<char> output;
        if (data.empty())
            return output;

        for (size_t i = 0; i < data.size(); ++i) {
            char count = 1;
            while (i + 1 < data.size() && data[i] == data[i + 1] && count < 127) {
                count++;
                i++;
            }
            output.push_back(count);
            output.push_back(data[i]);
        }
        return output;
    }

    static std::vector<char> decompress(const std::vector<char>& data) {
        std::vector<char> output;
        if (data.empty())
            return output;

        for (size_t i = 0; i < data.size(); i += 2) {
            if (i + 1 >= data.size())
                break;  // Malformed
            uint8_t count = (uint8_t)data[i];
            char val = data[i + 1];
            for (int j = 0; j < count; ++j) {
                output.push_back(val);
            }
        }
        return output;
    }
};

}  // namespace Network
