#ifndef ATTRCONTAINER_HPP
#define ATTRCONTAINER_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/tools/parser.hpp"
#include <map>
#include <string>
#include <type_traits>

namespace serializer {

/******************************************************************************/
/*                                member list                                 */
/******************************************************************************/

/// @brief Data structure that acts like a map with multiple types (one type per
///        node). It is used to store the identifiers and the references on the
///        attributes of the serialized class.
/// @param Conv Convertor used to serialize / deserialize the attribute.
/// @param Types Types of the attributes.
template <typename Conv, typename... Types> struct MemberList {
    /// @brief Empty serialize function (see specialization for non empty
    ///        parameter list for the non empty version).
    std::string serialize(std::string &str) const { return str; }

    /// @brief Empty deserialize function (see specialization for non empty
    ///        parameter list for the non empty version).
    void deserialize(std::string_view&) {}

    /// @brief We only use a default constructor here as we are in the case
    ///        where Types is empty (see the specialization for non empty
    ///        parameter list).
    MemberList() = default;
};

/// @brief Specialization of the MemberList. Handle the case when there is at
///        least one type.
template <typename Conv, typename H, typename... Types>
struct MemberList<Conv, H, Types...> {
    /* attributes *************************************************************/
    H &reference; ///< reference to an attribute to serialize
    Conv convertor; ///< instance of a convertor used to serialize the object
    MemberList<Conv, Types...> next; ///< next node of the list

    /* serialize **************************************************************/

    /// @brief Serialize the attribute hold by the current node.
    /// @param str String that will contain the result. We use the string to
    ///            manage an array of bytes that is modified by side effect.
    ///            This way, only one string is created.
    std::string serialize(std::string &str) const {
        convertor.serialize(reference, str);
        if constexpr (sizeof...(Types) > 0) {
            next.serialize(str);
        }
        return str;
    }

    /* deserialize ************************************************************/

    /// @brief Deserialize the attribute hold by the current node.
    /// @param str Reference to a string view of the string that holds the data.
    ///            We take a reference as the string view is modified during the
    ///            deserialization. It is used like an iterator but it is more
    ///            flexible.
    void deserialize(std::string_view &str) {
        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
                reference = nullptr;
            }
        }
        reference = convertor.deserialize(str, reference);
        next.deserialize(str);
    }

    /* constructor ************************************************************/

    /// @brief Constructor that initialize the current node and call the
    ///        constructor of the next node.
    /// @param head Reference to the attribute that will be managed by the
    ///             current node.
    /// @param types List of references to the other attributes (they will be
    ///              managed by other nodes).
    /// @param idsStr String that contains the identifiers of the attributes.
    MemberList(H &head, Types &...types)
        : reference(head), next(types...) {}
};

} // namespace serializer

#endif
