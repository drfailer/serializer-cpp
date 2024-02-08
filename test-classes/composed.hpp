#ifndef COMPOSED_HPP
#define COMPOSED_HPP
#include "serializer/serializer.hpp"
#include "test-classes/simple.hpp"

class Composed {
    SERIALIZABLE(Simple, int, double);

  public:
    Composed(const Simple &_s = Simple(0, 0), int _z = 0, double _w = 0.0)
        : SERIALIZER(s, z, w), s(_s), z(_z), w(_w) {}
    Composed(const Composed &other)
        : SERIALIZER(s, z, w), s(other.s), z(other.z), w(other.w) {}
    ~Composed() {}

    /* accessors **************************************************************/
    const Simple &getS() const { return s; }
    int getZ() const { return z; }
    double getW() const { return w; }
    void setT(const Simple &newT) { s = newT; }
    void setZ(int newZ) { z = newZ; }
    void setW(double newW) { w = newW; }

    /* copy *******************************************************************/
    Composed operator=(const Composed &other) {
        s = other.s;
        z = other.z;
        w = other.w;
        return *this;
    }

  private:
    Simple s;
    int z;
    double w;
};

inline bool operator==(const Composed &lhs, const Composed &rhs) {
    return lhs.getS() == rhs.getS() && lhs.getZ() == rhs.getZ() &&
           lhs.getW() == rhs.getW();
}

inline bool operator!=(const Composed &lhs, const Composed &rhs) {
    return !(lhs == rhs);
}

#endif
