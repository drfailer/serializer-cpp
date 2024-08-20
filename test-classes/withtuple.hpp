#ifndef WITH_TUPLE_HPP
#define WITH_TUPLE_HPP
#include "test-classes/composed.hpp"
#include "test-classes/simple.hpp"
#include <map>
#include <serializer/serialize.hpp>
#include <set>
#include <string>
#include <tuple>
#include <vector>

class WithTuple {
  public:
    explicit WithTuple(
        int i1 = 0, int i2 = 0, double d1 = 0, const std::string &str1 = "",
        const std::string &str2 = "", const std::string &str3 = "",
        const Simple &simple = Simple(), const Composed &composed = Composed(),
        const std::vector<int> &v = {}, const std::set<std::string> &s = {},
        std::map<std::string, std::string> m = {})
        : numberTuple_(i1, i2, d1), stringTuple_(str1, str2, str3),
          objTuple_(simple, composed), containerTuple_(v, s, m) {}
    ~WithTuple() = default;

    SERIALIZE(numberTuple_, stringTuple_, objTuple_, containerTuple_);

    /* accessors **************************************************************/
    [[nodiscard]] const std::tuple<std::string, std::string, std::string> &
    stringTuple() const {
        return stringTuple_;
    }
    [[nodiscard]] const std::tuple<int, int, double> &numberTuple() const {
        return numberTuple_;
    }
    [[nodiscard]] const std::tuple<Simple, Composed> &objTuple() const {
        return objTuple_;
    }
    [[nodiscard]] const std::tuple<std::vector<int>, std::set<std::string>,
                                   std::map<std::string, std::string>> &
    containerTuple() const {
        return containerTuple_;
    }

  private:
    std::tuple<int, int, double> numberTuple_;
    std::tuple<std::string, std::string, std::string> stringTuple_;
    std::tuple<Simple, Composed> objTuple_;
    std::tuple<std::vector<int>, std::set<std::string>,
               std::map<std::string, std::string>>
        containerTuple_;
};

#endif
