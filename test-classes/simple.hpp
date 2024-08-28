#ifndef SIMPLE_HPP
#define SIMPLE_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>

class Simple {
  public:
    explicit Simple(int x = 0, int y = 0, std::string str = "")
        : x_(x), y_(y), str_(std::move(str)) {}
    ~Simple() = default;

    SERIALIZE(x_, y_, str_);

    /* accessors **************************************************************/
    void y(int y) { this->y_ = y; }
    void x(int x) { this->x_ = x; }
    void str(const std::string &str) { this->str_ = str; }
    [[nodiscard]] int y() const { return y_; }
    [[nodiscard]] int x() const { return x_; }
    [[nodiscard]] std::string const &str() const { return str_; }
    [[nodiscard]] int &getY() { return y_; }
    [[nodiscard]] int &getX() { return x_; }
    [[nodiscard]] std::string &getStr() { return str_; }

  private:
    int x_;
    int y_;
    std::string str_;
};

inline bool operator==(const Simple &lhs, const Simple &rhs) {
    return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator!=(const Simple &lhs, const Simple &rhs) {
    return !(lhs == rhs);
}

#endif
