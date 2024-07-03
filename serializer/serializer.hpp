#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include "attrcontainer.hpp"
#include "serializer/convertor/convertor.hpp"
#include <fstream>
#include <sstream>

/* WARN: don't move this */
#include "serializable.hpp"

namespace serializer {

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

/// @brief Serializer class that is used to define the serializer attribute in
///        the serializable classes. Contains an attribute container that holds
///        references to the attributes of the serialized class.
/// @param Conv Convertor type (Convertor<> by default, but can be changed if a
///        custom convertor is used).
/// @param Types Types of the attributes to serialize.
template <typename Conv, typename... Types> class Serializer {
  public:
    /* constructor & destructor ***********************************************/

    /// @brief Constructor that is used to initialize the attribute container
    ///        with the reference to the serialized attributes.
    /// @param args References to the attributes to serialize.
    /// @param idsStr String containing the list of the attributes identifiers
    ///               ("x, y, z, angle"). It should be generated automatically
    ///               when the SERIALIZER macro is used.
    /// @param className Name of the serialized class (required for handeling
    ///                  polymorphic classes). It is obtained using the RTTI.
    Serializer(Types &...args, std::string const &idsStr, std::string className)
        : container(args..., idsStr), className(std::move(className)) {}

    /// @brief Default constructor to create an empty serialzer.
    Serializer() : container() {}

    /// @brief Default destructor (no memory allocation in this class).
    ~Serializer() = default;

    /* serialize **************************************************************/

    /// @brief Default serialize function (serialize the object).
    [[nodiscard]] std::string serialize() const {
        std::string str;
        return serialize(str);
    }

    /// @brief Serialize function with input string. It used instead of the
    ///        default version for optimization during the serialization (only
    ///        one string is created for serializing an object).
    std::string &serialize(std::string &str) const {
        auto size = className.size();
        str.append(reinterpret_cast<char*>(&size), sizeof(size));
        str.append(className);
        container.serialize(str);
        return str;
    }

    /// @brief Serialize the object and put the result into a file.
    void serializeFile(const std::string &fileName) const {
        std::ofstream file(fileName, std::ios::binary);
        file << serialize() << std::endl;
    }

    /* deserialize  ***********************************************************/

    /// @brief Default deserialize function that inits the deserialization.
    void deserialize(const std::string &str) {
        std::string_view strv = str;
        deserialize(strv);
    }

    /// @brief Deserialize function that is used during the deserialization. It
    ///        takes a string_view that is used pointer on the string and that
    ///        is modified during the deserialization process (can be seen as a
    ///        more flexible iterator).
    void deserialize(std::string_view &str) {
        using size_type = typename std::string::size_type;
        auto size = *reinterpret_cast<const size_type*>(str.data());
        str = str.substr(size + sizeof(size));
        container.deserialize(str);
    }

    /// @brief Deserialize function that reads the input from a file.
    void deserializeFile(const std::string &fileName) {
        std::ifstream file(fileName, std::ios::binary);
        std::ostringstream oss;
        oss << file.rdbuf();
        deserialize(oss.str()); // NOTE: it would be nice to be able to read the
                                // buffer directly (maybe faster than strings)
    }

  private:
    AttrContainer<Conv, Types...> container;
    std::string className;
};

};

#endif
