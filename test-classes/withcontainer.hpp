#ifndef WITHCONTAINER_HPP
#define WITHCONTAINER_HPP
#include "serializer/serializer.hpp"
#include "test-classes/simple.hpp"
#include <list>
#include <vector>

class WithContainer {
        SERIALIZABLE(std::vector<int>, std::vector<int>, std::list<double>, std::vector<Simple>);
  public:
    WithContainer() : SERIALIZER(emptyVec, vec, lst, classVec) {}
    ~WithContainer() = default;

    /* accessors **************************************************************/
    void addSimple(Simple simple) { classVec.push_back(simple); }
    void addDouble(double d) { lst.push_back(d); }
    void addInt(int i) { vec.push_back(i); }

    const std::vector<int> &getEmptyVec() const { return emptyVec; }
    const std::vector<int> &getVec() const { return vec; }
    const std::list<double> &getLst() const { return lst; }
    const std::vector<Simple> &getClassVec() const { return classVec; }

  private:
    std::vector<int> emptyVec;
    std::vector<int> vec;
    std::list<double> lst;
    std::vector<Simple> classVec;
};

#endif
