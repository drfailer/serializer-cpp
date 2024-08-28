#ifndef WITH_DYNAMIC_ARRAYS_HPP
#define WITH_DYNAMIC_ARRAYS_HPP
#include "simple.hpp"
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include <serializer/tools/dynamic_array.hpp>

class WithDynamicArray {
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

    SERIALIZE(borrowedSize_, multipleDimSize1_,
        SER_DARR(own_, 5), // own_
              SER_DARR(null_, 5),                                  // null_
              SER_DARR(null2_, 5, 5),                              // null2_
              SER_DARR(borrowed_, borrowedSize_),                  // borrowed_
              SER_DARR(ownSimple_, 5),                             // ownSimple_
              SER_DARR(multipleDim_, multipleDimSize1_, 2), // multipleDim_
              SER_DARR(twoDOneD_, 4, 4)                     // twoDOneD_
    );

    /* accessors **************************************************************/
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
