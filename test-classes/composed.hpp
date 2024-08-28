#ifndef COMPOSED_HPP
#define COMPOSED_HPP
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include "test-classes/simple.hpp"

class Composed {
  public:
    explicit Composed(const Simple &s = Simple(0, 0), int z = 0, double w = 0.0)
        : s_(s), z_(z), w_(w) {}
    ~Composed() = default;

    SERIALIZE(s_, z_, w_);

    /* accessors **************************************************************/
    [[nodiscard]] const Simple &s() const { return s_; }
    [[nodiscard]] int z() const { return z_; }
    [[nodiscard]] double w() const { return w_; }
    void s(const Simple &s) { s_ = s; }
    void z(int z) { z_ = z; }
    void w(double w) { w_ = w; }

  private:
    Simple s_;
    int z_;
    double w_;
};

inline bool operator==(const Composed &lhs, const Composed &rhs) {
    return lhs.s() == rhs.s() && lhs.z() == rhs.z() && lhs.w() == rhs.w();
}

inline bool operator!=(const Composed &lhs, const Composed &rhs) {
    return !(lhs == rhs);
}

#endif
