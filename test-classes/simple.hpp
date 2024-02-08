#ifndef SIMPLE_HPP
#define SIMPLE_HPP
#include "serializer/serializer.hpp"

class Simple {
    SERIALIZABLE(int, int);
  public:
    Simple(int _x = 0, int _y = 0) : SERIALIZER(x, y), x(_x), y(_y) {}
    Simple(const Simple &other) : Simple(other.x, other.y) {}
    ~Simple() = default;

    /* copy *******************************************************************/
    Simple operator=(const Simple &other) { // this is required for deserialization
        x = other.x;
        y = other.y;
        return *this;
    }

    /* accessors **************************************************************/
    void setY(int y) { this->y = y; }
    int getY() const { return y; }
    void setX(int x) { this->x = x; }
    int getX() const { return x; }

  private:
    int x;
    int y;
};

inline bool operator==(const Simple& lhs, const Simple& rhs) {
    return lhs.getX() == rhs.getX() && lhs.getY() == rhs.getY();
}

inline bool operator!=(const Simple &lhs, const Simple &rhs) {
    return !(lhs == rhs);
}

#endif
