#ifndef WITHCONTAINER_HPP
#define WITHCONTAINER_HPP
#include <serializer/serialize.hpp>
#include "test-classes/simple.hpp"
#include <array>
#include <list>
#include <vector>

class WithContainer {
  public:
     SERIALIZE(emptyVec, vec, lst, classVec, vec2D, arr, arrPtr, arrSimple);

    /* accessors **************************************************************/
    void addSimple(const Simple &simple) { classVec.push_back(simple); }
    void addDouble(double d) { lst.push_back(d); }
    void addInt(int i) { vec.push_back(i); }
    void addVec(std::vector<int> &&vecInt) { vec2D.push_back(vecInt); }
    void addArr(int i, int n) { arr[i] = n; }
    void addArrPtr(int i, int* n) { arrPtr[i] = n; }
    void addArrSimple(int i, Simple n) { arrSimple[i] = n; }

    [[nodiscard]] const std::vector<int> &getEmptyVec() const {
        return emptyVec;
    }
    [[nodiscard]] const std::vector<int> &getVec() const { return vec; }
    [[nodiscard]] const std::list<double> &getLst() const { return lst; }
    [[nodiscard]] const std::vector<Simple> &getClassVec() const {
        return classVec;
    }
    [[nodiscard]] const std::vector<std::vector<int>> &getVec2D() const {
        return vec2D;
    }
    [[nodiscard]] const std::array<int, 10> &getArr() const { return arr; }
    [[nodiscard]] const std::array<int*, 10> &getArrPtr() const { return arrPtr; }
    [[nodiscard]] const std::array<Simple, 10> &getArrSimple() const {
      static_assert(!serializer::tools::concepts::Trivial<Simple>);
      return arrSimple;
    }

  private:
    std::vector<int> emptyVec = {};
    std::vector<int> vec = {};
    std::list<double> lst = {};
    std::vector<Simple> classVec = {};
    std::vector<std::vector<int>> vec2D = {};
    std::array<int, 10> arr = {};
    std::array<int*, 10> arrPtr = {};
    std::array<Simple, 10> arrSimple;
};

#endif
