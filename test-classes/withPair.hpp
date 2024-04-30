#ifndef WITHPAIR_HPP
#define WITHPAIR_HPP
#include "serializer/serializer.hpp"
#include "test-classes/composed.hpp"
#include "test-classes/simple.hpp"
#include <set>
#include <string>
#include <utility>
#include <vector>

class WithPair {
    SERIALIZABLE(std::pair<int, int>, std::pair<std::string, std::string>,
                 std::pair<Simple, Composed>,
                 std::pair<std::vector<int>, std::set<std::string>>);

  public:
    WithPair(int i1 = 0, int i2 = 0, std::string str1 = "",
             std::string str2 = "", const Simple &simple = Simple(),
             const Composed &composed = Composed(),
             const std::vector<int> &v = {},
             const std::set<std::string> &s = {})
        : SERIALIZER(intPair, stringPair, objPair, containerPair),
          intPair(std::make_pair(i1, i2)),
          stringPair(std::make_pair(str1, str2)),
          objPair(std::make_pair(simple, composed)), containerPair(v, s) {}
    ~WithPair() = default;

    /* accessors **************************************************************/
    const std::pair<std::string, std::string> &getStringPair() const {
        return stringPair;
    }
    const std::pair<int, int> &getIntPair() const { return intPair; }
    const std::pair<Simple, Composed> &getObjPair() const { return objPair; }
    const std::pair<std::vector<int>, std::set<std::string>> &
    getContainerPair() const {
        return containerPair;
    }

  private:
    std::pair<int, int> intPair;
    std::pair<std::string, std::string> stringPair;
    std::pair<Simple, Composed> objPair;
    std::pair<std::vector<int>, std::set<std::string>> containerPair;
};

#endif
