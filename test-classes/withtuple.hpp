#ifndef WITHTUPLE_HPP
#define WITHTUPLE_HPP
#include "serializer/serializer.hpp"
#include "test-classes/composed.hpp"
#include "test-classes/simple.hpp"
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

class WithTuple {
    SERIALIZABLE(std::tuple<int, int, double>,
                 std::tuple<std::string, std::string, std::string>,
                 std::tuple<Simple, Composed>,
                 std::tuple<std::vector<int>, std::set<std::string>,
                            std::map<std::string, std::string>>);

  public:
    WithTuple(int i1 = 0, int i2 = 0, double d1 = 0,
              const std::string &str1 = "", const std::string &str2 = "",
              const std::string &str3 = "", const Simple &simple = Simple(),
              const Composed &composed = Composed(),
              const std::vector<int> &v = {},
              const std::set<std::string> &s = {},
              std::map<std::string, std::string> m = {})
        : SERIALIZER(numberTuple, stringTuple, objTuple, containerTuple),
          numberTuple(i1, i2, d1), stringTuple(str1, str2, str3),
          objTuple(simple, composed), containerTuple(v, s, m) {}
    ~WithTuple() = default;

    /* accessors **************************************************************/
    const std::tuple<std::string, std::string, std::string> &
    getStringTuple() const {
        return stringTuple;
    }
    const std::tuple<int, int, double> &getNumberTuple() const { return numberTuple; }
    const std::tuple<Simple, Composed> &getObjTuple() const { return objTuple; }
    const std::tuple<std::vector<int>, std::set<std::string>,
                     std::map<std::string, std::string>> &
    getContainerTuple() const {
        return containerTuple;
    }

  private:
    std::tuple<int, int, double> numberTuple;
    std::tuple<std::string, std::string, std::string> stringTuple;
    std::tuple<Simple, Composed> objTuple;
    std::tuple<std::vector<int>, std::set<std::string>,
               std::map<std::string, std::string>>
        containerTuple;
};

#endif
