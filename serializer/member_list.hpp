#ifndef MEMBER_LIST_HPP
#define MEMBER_LIST_HPP
#include "tools/metafunctions.hpp"
#include "tools/ml_arg_type.hpp"
#include <functional>
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
    void deserialize(std::string_view &) {}

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
    std::function<void(bool, std::string &)> func;
    tools::mtf::ml_arg_type_t<H>
        reference;  ///< reference to an attribute to serialize
    Conv convertor; ///< instance of a convertor used to serialize the object
    MemberList<Conv, Types...> next; ///< next node of the list

    /* serialize **************************************************************/

    /// @brief Serialize the attribute hold by the current node.
    /// @param str String that will contain the result. We use the string to
    ///            manage an array of bytes that is modified by side effect.
    ///            This way, only one string is created.
    std::string serialize(std::string &str) const {
        if constexpr (tools::mtf::is_function_v<H>) {
            reference(Phases::Serialization, str);
        } else {
            convertor.serialize_(reference, str);
        }
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
        if constexpr (tools::mtf::is_function_v<H>) {
            reference(Phases::Deserialization, str);
        } else if constexpr (std::is_array_v<H> ||
                             tools::mtf::is_dynamic_array_v<H>) {
            // static arrays can't be assigned
            convertor.deserialize_(str, reference);
        } else {
            reference = std::move(convertor.deserialize_(str, reference));
        }
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
    explicit MemberList(tools::mtf::ml_arg_type_t<H> head,
                        tools::mtf::ml_arg_type_t<Types>... types)
        : reference(head), next(types...) {}
};

} // namespace serializer

#endif
