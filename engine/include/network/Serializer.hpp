#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace Network {

class Serializer {
public:
    Serializer() = default;

    // Get the underlying buffer
    const std::vector<char>& getBuffer() const { return buffer_; }

    // Write a POD type
    template <typename T>
    void write(const T& value) {
        static_assert(std::is_standard_layout<T>::value, "Data must be standard layout");
        const char* ptr = reinterpret_cast<const char*>(&value);
        buffer_.insert(buffer_.end(), ptr, ptr + sizeof(T));
    }

    // Write raw bytes
    void writeBytes(const char* data, size_t size) {
        buffer_.insert(buffer_.end(), data, data + size);
    }

    // Write std::string (length + chars)
    void writeString(const std::string& str) {
        uint32_t len = static_cast<uint32_t>(str.size());
        write(len);
        writeBytes(str.data(), len);
    }

private:
    std::vector<char> buffer_;
};

class Deserializer {
public:
    Deserializer(const std::vector<char>& data) : buffer_(data), offset_(0) {}
    Deserializer(const char* data, size_t size) : buffer_(data, data + size), offset_(0) {}

    // Read a POD type
    template <typename T>
    T read() {
        static_assert(std::is_standard_layout<T>::value, "Data must be standard layout");
        if (offset_ + sizeof(T) > buffer_.size()) {
            throw std::runtime_error("Deserializer: Buffer underflow");
        }
        T value;
        std::memcpy(&value, buffer_.data() + offset_, sizeof(T));
        offset_ += sizeof(T);
        return value;
    }

    // Read raw bytes
    std::vector<char> readBytes(size_t size) {
        if (offset_ + size > buffer_.size()) {
            throw std::runtime_error("Deserializer: Buffer underflow");
        }
        std::vector<char> out(buffer_.data() + offset_, buffer_.data() + offset_ + size);
        offset_ += size;
        return out;
    }

    // Read std::string
    std::string readString() {
        uint32_t len = read<uint32_t>();
        if (offset_ + len > buffer_.size()) {
            throw std::runtime_error("Deserializer: Buffer underflow for string");
        }
        std::string str(buffer_.data() + offset_, len);
        offset_ += len;
        return str;
    }

    // Check if more data is available
    bool hasData() const { return offset_ < buffer_.size(); }

private:
    std::vector<char> buffer_;
    size_t offset_;
};

}  // namespace Network
