#ifndef CSTRUCT_H
#define CSTRUCT_H
#include <serializer/serialize.hpp>

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
        : c(c_), i(i_), l(l_), f(f_), d(d_) {}

    SERIALIZE(serializer::tools::mtf::type_list<char, int, long, float, double>(), c, i, l, f, d);
    char c = 0;
    int i = 0;
    long l = 0;
    float f = 0;
    double d = 0;
};

#endif // SERIALIZER_CSTRUCT_H
