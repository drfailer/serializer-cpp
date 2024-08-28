#ifndef WITH_SET_HPP
#define WITH_SET_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>
#include <set>
#include <string>

class WithSet {
  public:
    WithSet() = default;
    ~WithSet() = default;

    SERIALIZE(set_);

    /* accessors **************************************************************/
    void insert(const std::string &elt) { set_.insert(elt); }

    [[nodiscard]] const std::set<std::string> &set() const { return set_; }

  private:
    std::set<std::string> set_;
};

#endif
