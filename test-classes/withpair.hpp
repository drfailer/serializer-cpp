#ifndef WITH_PAIR_HPP
#define WITH_PAIR_HPP
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include "test-classes/composed.hpp"
#include "test-classes/simple.hpp"
#include <set>
#include <string>
#include <utility>
#include <vector>

class WithPair {
  public:
    explicit WithPair(int i1 = 0, int i2 = 0, std::string str1 = "",
                      std::string str2 = "", const Simple &simple = Simple(),
                      const Composed &composed = Composed(),
                      const std::vector<int> &v = {},
                      const std::set<std::string> &s = {})
        : intPair_(std::make_pair(i1, i2)),
          stringPair_(std::make_pair(std::move(str1), std::move(str2))),
          objPair_(std::make_pair(simple, composed)), containerPair_(v, s) {}
    ~WithPair() = default;

    SERIALIZE(intPair_, stringPair_, objPair_, containerPair_);

    /* accessors **************************************************************/
    [[nodiscard]] const std::pair<std::string, std::string> &
    stringPair() const {
        return stringPair_;
    }
    [[nodiscard]] const std::pair<int, int> &intPair() const {
        return intPair_;
    }
    [[nodiscard]] const std::pair<Simple, Composed> &objPair() const {
        return objPair_;
    }
    [[nodiscard]] const std::pair<std::vector<int>, std::set<std::string>> &
    getContainerPair() const {
        return containerPair_;
    }

  private:
    std::pair<int, int> intPair_;
    std::pair<std::string, std::string> stringPair_;
    std::pair<Simple, Composed> objPair_;
    std::pair<std::vector<int>, std::set<std::string>> containerPair_;
};

#endif
