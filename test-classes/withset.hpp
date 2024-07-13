#ifndef WITH_SET_HPP
#define WITH_SET_HPP
#include <serializer/serializable.hpp>
#include <set>
#include <string>

class WithSet {
    SERIALIZABLE(std::set<std::string>);

  public:
    WithSet() : SERIALIZER(set_) {}
    ~WithSet() = default;

    /* accessors **************************************************************/
    void insert(const std::string &elt) { set_.insert(elt); }

    [[nodiscard]] const std::set<std::string> &set() const { return set_; }

  private:
    std::set<std::string> set_;
};

#endif
