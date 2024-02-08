#ifndef WITHPAIR_HPP
#define WITHPAIR_HPP
#include "serializer/serializer.hpp"
#include <string>
#include <utility>

class WithPair {
    SERIALIZABLE(std::pair<int, int>, std::pair<std::string, std::string>);

  public:
    const std::pair<std::string, std::string> &getStringPair() const {
        return stringPair;
    }
    const std::pair<int, int> &getIntPair() const { return intPair; }
    WithPair(int i1 = 0, int i2 = 0, std::string str1 = "",
             std::string str2 = "")
        : SERIALIZER(intPair, stringPair), intPair(std::make_pair(i1, i2)),
          stringPair(std::make_pair(str1, str2)) {}
    ~WithPair() = default;

  private:
    std::pair<int, int> intPair;
    std::pair<std::string, std::string> stringPair;
};

#endif
