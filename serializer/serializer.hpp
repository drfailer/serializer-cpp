#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP
#include "member_list.hpp"
#include <fstream>
#include <sstream>

namespace serializer {

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

/// @brief Serializer class that is used to define the serializer attribute in
///        the serializable classes. Contains an attribute members_ that holds
///        references to the attributes of the serialized class.
/// @param Conv Convertor type (Convertor<> by default, but can be changed if a
///        custom convertor is used).
/// @param Types Types of the attributes to serialize.
template <typename Conv, typename... Types> class Serializer {
  public:
    /* constructor & destructor ***********************************************/

    /// @brief Constructor that is used to initialize the attribute members_
    ///        with the reference to the serialized attributes.
    /// @param args References to the attributes to serialize.
    /// @param idsStr String containing the list of the attributes identifiers
    ///               ("x_, y_, z, angle"). It should be generated automatically
    ///               when the SERIALIZER macro is used.
    /// @param className Name of the serialized class (required for handeling
    ///                  polymorphic classes). It is obtained using the RTTI.
    explicit Serializer(tools::mtf::ml_arg_type_t<Types> ...args, std::string className)
        : members_(args...), className_(std::move(className)) {}

    /// @brief Default constructor to create an empty serialzer.
    Serializer() : members_() {}

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
        auto size = className_.size();
        str.append(reinterpret_cast<char*>(&size), sizeof(size));
        str.append(className_);
        members_.serialize(str);
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
        members_.deserialize(str);
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
    MemberList<Conv, Types...> members_; ///< allow access to the members
    std::string className_; ///< used for deserializing polymorphic objects
};

};

#endif
