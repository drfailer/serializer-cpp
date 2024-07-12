#ifndef SIMPLE_HPP
#define SIMPLE_HPP
#include "serializer/serializable.hpp"

class Simple {
    SERIALIZABLE(int, std::string, int);

  public:
    explicit Simple(int x = 0, int y = 0, std::string str = "")
        : SERIALIZER(x_, str_, y_), x_(x), y_(y), str_(std::move(str)) {}
    Simple(const Simple &other) : Simple(other.x_, other.y_) {}
    ~Simple() = default;

    /* copy *******************************************************************/
    Simple &
    operator=(const Simple &other) { // this is required for deserialization
        x_ = other.x_;
        y_ = other.y_;
        return *this;
    }

    /* accessors **************************************************************/
    void y(int y) { this->y_ = y; }
    void x(int x) { this->x_ = x; }
    void str(const std::string &str) { this->str_ = str; }
    [[nodiscard]] int y() const { return y_; }
    [[nodiscard]] int x() const { return x_; }
    [[nodiscard]] std::string const &str() const { return str_; }

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
