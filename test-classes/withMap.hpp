#ifndef WITH_MAP_HPP
#define WITH_MAP_HPP
#include "test-classes/simple.hpp"
#include <map>
#include <string>

class WithMap {
    SERIALIZABLE(std::map<std::string, std::string>);

  public:
    WithMap() : SERIALIZER(map_) {}
    ~WithMap() = default;

    /* accessors **************************************************************/
    void insert(const std::string &key, const std::string &value) {
        map_.insert(std::make_pair(key, value));
    }

    [[nodiscard]] const std::map<std::string, std::string> &map() const {
        return map_;
    }

  private:
    std::map<std::string, std::string> map_;
};

#endif
