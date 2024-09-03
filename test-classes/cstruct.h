#ifndef CSTRUCT_H
#define CSTRUCT_H
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>

struct CStruct {
    char c;
    int i;
    long l;
    float f;
    double d;
};

class CStructSerializable {
  public:
    explicit CStructSerializable(char c_ = 0, int i_ = 0, long l_ = 0,
                                 float f_ = 0, double d_ = 0)
        : c_(c_), i_(i_), l_(l_), f_(f_), d_(d_) {}

    char c() const { return c_; }
    int i() const { return i_; }
    long l() const { return l_; }
    float f() const { return f_; }
    double d() const { return d_; }

    SERIALIZE(c_, i_, l_, f_, d_);

  private:
    char c_ = 0;
    int i_ = 0;
    long l_ = 0;
    float f_ = 0;
    double d_ = 0;
};

#endif // SERIALIZER_CSTRUCT_H
