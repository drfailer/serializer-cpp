#ifndef WITH_STRING_HPP
#define WITH_STRING_HPP
#include "serializer/serializer.hpp"
#include <string>
#include <utility>

class WithString {
    SERIALIZABLE(int, std::string);

  public:
    WithString(int x, std::string str)
        : SERIALIZER(x_, str_), x_(x), str_(std::move(str)) {}
    ~WithString() = default;

    /* accessors **************************************************************/
    void str(std::string str) { this->str_ = std::move(str); }
    void x(int x) { this->x_ = x; }
    [[nodiscard]] std::string str() const { return str_; }
    [[nodiscard]] int x() const { return x_; }

  private:
    int x_;
    std::string str_;
};

#endif
