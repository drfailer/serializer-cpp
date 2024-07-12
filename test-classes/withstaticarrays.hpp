#ifndef WITHSTATICARRAYS_HPP
#define WITHSTATICARRAYS_HPP
#include "test-classes/simple.hpp"
#include <serializer/serializable.hpp>

class WithStaticArrays {
    SERIALIZABLE(int[10], int[10][10], int[10][10][2], Simple[2],
                 Simple[2][2]);

  public:
    WithStaticArrays():
      SERIALIZER(arr_, grid_, tensor_, arrSimple_, gridSimple_) {}

    /* accessors
     * ****************************************************************/
    int &arr(size_t i) { return arr_[i]; }
    int &grid(size_t i, size_t j) { return grid_[i][j]; }
    int &tensor(size_t i, size_t j, size_t k) { return tensor_[i][j][k]; }
    Simple &arrSimple(size_t i) { return arrSimple_[i]; }
    Simple &gridSimple(size_t i, size_t j) { return gridSimple_[i][j]; }

  private:
    int arr_[10];
    int grid_[10][10];
    int tensor_[10][10][2];
    Simple arrSimple_[2];
    Simple gridSimple_[2][2];
};

#endif
