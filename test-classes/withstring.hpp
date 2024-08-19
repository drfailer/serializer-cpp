#ifndef WITH_STRING_HPP
#define WITH_STRING_HPP
#include <serializer/serialize.hpp>
#include <string>
#include <utility>

class WithString {
  public:
    WithString(int x, std::string str)
        : x_(x), str_(std::move(str)) {}
    ~WithString() = default;

    SERIALIZE(x_, str_);

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
