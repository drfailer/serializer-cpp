#ifndef WITHMAP_HPP
#define WITHMAP_HPP
#include "test-classes/simple.hpp"
#include <map>
#include <string>

class WithMap {
    SERIALIZABLE(std::map<std::string, std::string>);

  public:
    WithMap() : SERIALIZER(map) {}
    ~WithMap() = default;

    /* accessors **************************************************************/
    void insert(const std::string &key, const std::string &value) {
        map.insert(std::make_pair(key, value));
    }

    const std::map<std::string, std::string> &getMap() const { return map; }

  private:
    std::map<std::string, std::string> map;
};

#endif
