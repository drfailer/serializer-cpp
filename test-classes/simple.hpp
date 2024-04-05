#ifndef SIMPLE_HPP
#define SIMPLE_HPP
#include "serializer/serializer.hpp"

class Simple {
    SERIALIZABLE(int, std::string, int);

  public:
    Simple(int _x = 0, int _y = 0, const std::string &_str = "")
        : SERIALIZER(x, str, y), x(_x), y(_y), str(_str) {}
    Simple(const Simple &other) : Simple(other.x, other.y) {}
    ~Simple() = default;

    /* copy *******************************************************************/
    Simple
    operator=(const Simple &other) { // this is required for deserialization
        x = other.x;
        y = other.y;
        return *this;
    }

    /* accessors **************************************************************/
    void setY(int y) { this->y = y; }
    int getY() const { return y; }
    void setX(int x) { this->x = x; }
    int getX() const { return x; }
    const std::string &getStr() const { return str; }
    void setStr(const std::string &str) { this->str = str; }

  private:
    int x;
    int y;
    std::string str;
};

inline bool operator==(const Simple &lhs, const Simple &rhs) {
    return lhs.getX() == rhs.getX() && lhs.getY() == rhs.getY();
}

inline bool operator!=(const Simple &lhs, const Simple &rhs) {
    return !(lhs == rhs);
}

#endif
