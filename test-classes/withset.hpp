#ifndef WITHSET_HPP
#define WITHSET_HPP
#include "test-classes/simple.hpp"
#include <set>
#include <string>

class WithSet {
    SERIALIZABLE(std::set<std::string>);

  public:
    WithSet() : SERIALIZER(set) {}
    ~WithSet() = default;

    /* accessors **************************************************************/
    void insert(const std::string &elt) { set.insert(elt); }

    const std::set<std::string> &getSet() const { return set; }

  private:
    std::set<std::string> set;
};

#endif
