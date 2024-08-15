#ifndef WITH_DYNAMIC_ARRAYS_HPP
#define WITH_DYNAMIC_ARRAYS_HPP
#include "simple.hpp"
#include <serializer/tools/dynamic_array.hpp>
#include <serializer/serialize.hpp>

class WithDynamicArray {
    using type_list = serializer::tools::mtf::type_list<
        size_t, size_t, SER_DARR_T(int *, size_t), // own_
        SER_DARR_T(int *, size_t),                 // null_
        SER_DARR_T(int **, size_t, size_t),        // null2_
        SER_DARR_T(double *, size_t &),            // borrowed_
        SER_DARR_T(Simple *, size_t),              // ownSimple_
        SER_DARR_T(int **, size_t &, size_t),      // multipleDim_
        SER_DARR_T(int *, size_t, size_t)          // twoDOneD_
        >;

  public:
    explicit WithDynamicArray(size_t multipleDimSize1 = 0)
        : own_(new int[5]), null2_(new int *[5]), ownSimple_(new Simple[5]),
          multipleDim_(multipleDimSize1 == 0 ? nullptr
                                             : new int *[multipleDimSize1]),
          twoDOneD_(new int[4 * 4]), multipleDimSize1_(multipleDimSize1) {
        for (size_t i = 0; i < 5; ++i) {
            null2_[i] = nullptr;
        }

        if (multipleDim_) {
            for (size_t i = 0; i < multipleDimSize1; ++i) {
                multipleDim_[i] = new int[2];
            }
        }
    }

    ~WithDynamicArray() {
        delete[] own_;
        // null_ sould be nullptr
        delete[] null2_; // the sub pointers should be nullptr
        delete[] ownSimple_;
        delete[] twoDOneD_;

        if (multipleDim_) {
            for (size_t i = 0; i < multipleDimSize1_; ++i) {
                delete[] multipleDim_[i];
            }
        }
        delete[] multipleDim_;
    }

    /* SERIALIZE(type_list(), borrowedSize_, multipleDimSize1_, */
    /*           SER_DARR_T(int *, size_t)(own_, 5),               // own_ */
    /*           SER_DARR_T(int *, size_t)(null_, 5),              // null_ */
    /*           SER_DARR_T(int **, size_t, size_t)(null2_, 5, 5), // null2_ */
    /*           SER_DARR_T(double *, size_t &)(borrowed_, */
    /*                                          borrowedSize_), // borrowed_ */
    /*           SER_DARR_T(Simple *, size_t)(ownSimple_, 5),   // ownSimple_ */
    /*           SER_DARR_T(int **, size_t &, size_t)(multipleDim_, */
    /*                                                multipleDimSize1_, */
    /*                                                2),           // multipleDim_ */
    /*           SER_DARR_T(int *, size_t, size_t)(twoDOneD_, 4, 4) // twoDOneD_ */
    /* ); */

    /* accessors
     * ****************************************************************/
    void borrow(double *borrowed, size_t size) {
        this->borrowed_ = borrowed;
        this->borrowedSize_ = size;
    }
    int *own() { return own_; }
    int *null() { return null_; }
    int **null2() { return null2_; }
    double *borrowed() { return borrowed_; }
    Simple *ownSimple() { return ownSimple_; }
    int **multipleDim() { return multipleDim_; }
    int *twoDOneD() { return twoDOneD_; }

  private:
    int *own_ = nullptr;
    int *null_ = nullptr;
    int **null2_ = nullptr;
    double *borrowed_ = nullptr;
    Simple *ownSimple_ = nullptr;
    int **multipleDim_ = nullptr;
    int *twoDOneD_ = nullptr;
    size_t multipleDimSize1_ = 0;
    size_t borrowedSize_ = 0;
};

#endif
