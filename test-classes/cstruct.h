#ifndef SERIALIZER_CSTRUCT_H
#define SERIALIZER_CSTRUCT_H
#include <serializer/serializer.hpp>

struct CStruct {
    char c;
    int i;
    long l;
    float f;
    double d;
};

class CStructSerializable {
    SERIALIZABLE(char, int, long, float, double);

  public:
    CStructSerializable(char c_ = 0, int i_ = 0, long l_ = 0, float f_ = 0,
                        double d_ = 0)
        : SERIALIZER(c, i, l, f, d), c(c_), i(i_), l(l_), f(f_), d(d_) {}

    char c = 0;
    int i = 0;
    long l = 0;
    float f = 0;
    double d = 0;
};

#endif // SERIALIZER_CSTRUCT_H
