#ifndef WITH_C_STRUCT_H
#define WITH_C_STRUCT_H
#include <serializer/serializable.hpp>

/******************************************************************************/
/*                                with cstruct                                */
/******************************************************************************/

struct Struct1 {
    double d;
    char c;
    int arr[3];
};

class WithCStruct {
    SERIALIZABLE(int, serializer::tools::CStruct<Struct1>)
  public:
    WithCStruct(int i = 0, double d = 0, char c = 0, int i1 = 0, int i2 = 0,
                int i3 = 0)
        : SERIALIZER(i_, serializer::tools::CStruct<Struct1>(cstruct_)), i_(i),
          cstruct_({d, c, {i1, i2, i3}}) {}

    int i() const { return i_; }
    Struct1 cstruct() const { return cstruct_; }

  private:
    int i_;
    Struct1 cstruct_;
};

/******************************************************************************/
/*                              without cstruct                               */
/******************************************************************************/

class NotStruct {
    SERIALIZABLE(double, char, int[3]);

  public:
    NotStruct(double d = 0, char c = 0, int i1 = 0, int i2 = 0, int i3 = 0)
        : SERIALIZER(d_, c_, arr_), d_(d), c_(c), arr_{i1, i2, i3} {}
    NotStruct(NotStruct const &other)
        : NotStruct(other.d_, other.c_, other.arr_[0], other.arr_[1],
                    other.arr_[2]) {}

    NotStruct &operator=(NotStruct const &other) {
        if (&other == this) {
            return *this;
        }
        d_ = other.d_;
        c_ = other.c_;

        for (size_t i = 0; i < sizeof(arr_) / sizeof(arr_[0]); ++i) {
            arr_[i] = other.arr_[i];
        }
        return *this;
    }

    double d_;
    char c_;
    int arr_[3];
};

class WithoutCStruct {
    SERIALIZABLE(int, NotStruct);

  public:
    WithoutCStruct(int i = 0, double d = 0, char c = 0, int i1 = 0, int i2 = 0,
                   int i3 = 0)
        : SERIALIZER(i_, notcstruct_), i_(i), notcstruct_(d, c, i1, i2, i3) {}

    int i() const { return i_; }
    NotStruct notcstruct() const { return notcstruct_; }

  private:
    int i_;
    NotStruct notcstruct_;
};

#endif
