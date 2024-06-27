#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include "attrcontainer.hpp"
#include "convertor.hpp"
#include <fstream>
#include <sstream>

/* WARN: don't move this */
#include "serializable.hpp"

namespace serializer {

// NOTE: we should use static asserts to create useful error messages
// NOTE: removing the ' ' (spaces) in the serialized strings could be
// interesting

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

template <typename Conv, typename... Types> class Serializer {
  public:
    /* constructor & destructor ***********************************************/
    Serializer(Types &...args, std::string const &idsStr, std::string className)
        : container(args..., idsStr), className(std::move(className)) {}
    Serializer() : container() {}
    ~Serializer() = default;

    /* serialize **************************************************************/
    [[nodiscard]] std::string serialize() const {
        std::string str;
        return serialize(str);
    }

    // used to optimize the serialization of objects
    std::string &serialize(std::string &str) const {
        auto size = className.size();
        str.append(reinterpret_cast<char*>(&size), sizeof(size));
        str.append(className);
        container.serialize(str);
        return str;
    }

    void serializeFile(const std::string &fileName) const {
        std::ofstream file(fileName);
        file << serialize() << std::endl;
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string &str) {
        std::string_view strv = str;
        deserialize(strv);
    }

    void deserialize(std::string_view &str) {
        using size_type = typename std::string::size_type;
        auto size = *reinterpret_cast<const size_type*>(str.data());
        str = str.substr(size + sizeof(size));
        container.deserialize(str);
    }

    void deserializeFile(const std::string &fileName) {
        std::ifstream file(fileName);
        std::ostringstream oss;
        oss << file.rdbuf();
        deserialize(oss.str()); // NOTE: it would be nice to be able to read the
                                // buffer directly (may be faster than strings)
    }

  private:
    AttrContainer<Conv, Types...> container;
    std::string className;
};

};

#endif
