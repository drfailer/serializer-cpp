#ifndef WITH_MAP_HPP
#define WITH_MAP_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>
#include <map>
#include <string>

class WithMap {
  public:
    WithMap() {}
    ~WithMap() = default;

    SERIALIZE(map_);

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
