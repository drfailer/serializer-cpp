#ifndef SIMPLE_HPP
#define SIMPLE_HPP
#include "serializer/serializable.hpp"

class Simple : public Serializable<int, int> {
  public:
    Simple(int _x = 0, int _y = 0) : serializable(x, y), x(_x), y(_y) {}
    Simple(const Simple &other) : Simple(other.x, other.y) {}
    ~Simple() = default;

    /* copy *******************************************************************/
    Simple operator=(const Simple &other) {
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

#endif
