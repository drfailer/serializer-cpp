#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "convert.hpp"
#include "serializer/tools/concepts.hpp"
#include "serializer/tools/exceptions.hpp"
#include "serializer/tools/metafunctions.hpp"
#include "serializer/tools/parser.hpp"
#include "serializer/tools/tools.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/******************************************************************************/
/*                          default convertor class                           */
/******************************************************************************/

namespace serializer {

/// @brief Default convertor that contains the functions for serializing and
///        deserializing types from the standard library. It inherits from the
///        convert behavior for additional types so the user can add its own
///        functions.
/// @param AdditionalTypes External types for which the user can add support.
template <typename... AdditionalTypes>
struct Convertor : public Convert<AdditionalTypes>... {

    /* no automatic serialization types (custom convertor) ********************/

    /// @brief Fallback functions for non serializable types. Here either we use
    ///        a custom function if it has be implemented or we throw an
    ///        exception.
    ///        Note: exceptions are a way to control the display of the error
    ///        message. We could use a static assert but the message will be
    ///        lost in the hole compiler error output.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::NonSerializable T>
    std::string &serialize(T const &elt, std::string str) const {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            return this->serialize_(elt, str);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<T>();
        }
    }

    /// @brief Fallback functions for non deserializable types. Here either we
    ///        use a custom function if it has be implemented or we throw an
    ///        exception.
    ///        Note: exceptions are a way to control the display of the error
    ///        message. We could use a static assert but the message will be
    ///        lost in the hole compiler error output.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <serializer::concepts::NonDeserializable T>
    T deserialize(std::string_view &str, T &elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            return this->deserialize_(str, elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<T>();
        }
    }

    /* serializable ***********************************************************/

    /// @brief Serialize function for the serializable types (they have a
    ///        serialize method).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Serializable T>
    std::string &serialize(T const &elt, std::string &str) const {
        return str.append(elt.serialize());
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Deserializable T>
    std::remove_reference_t<T> deserialize(std::string_view &str, T &) {
        std::remove_reference_t<T> t;
        t.deserialize(str);
        return t;
    }

    /* fundamental types ******************************************************/

    /// @brief Serialize function for the fundamental types.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Fundamental T>
    std::string &serialize(T const &elt, std::string &str) const {
        return str.append(reinterpret_cast<const char *>(&elt), sizeof(T));
    }

    /// @brief Deserialize function for the deserializable fundamental types.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Fundamental T>
    T deserialize(std::string_view &str, T &) {
        T t = *reinterpret_cast<const T *>(str.data());
        str = str.substr(sizeof(T));
        return t;
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// TODO: will change when the pointer will be configurable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Pointer T>
    std::string &serialize(T const &elt, std::string &str) const {
        if (elt != nullptr) {
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {
                str.append(elt->serialize());
                return str;
            } else {
                return serialize<std::remove_pointer_t<T>>(*elt, str);
            }
        } else {
            return str.append("nullptr");
        }
    }

    /// @brief Deserialize function for the pointer types. If the pointer is not
    ///        null, a dynamic allocation is performed before deserializing the
    ///        result. This memory should be handled by the user.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::ConcretePtr T>
    std::remove_reference_t<T> deserialize(std::string_view &str, T &) {
        if (str.starts_with("nullptr")) {
            str = str.substr(7);
            return nullptr;
        }
        using Type = typename std::remove_pointer_t<std::remove_reference_t<T>>;
        Type *t = new Type();
        if constexpr (std::is_fundamental_v<Type>) {
            *t = deserialize(str, *t);
        } else {
            t->deserialize(str);
        }
        return t;
    }

    /* smart pointers *********************************************************/

    /// @brief Serialize function for smart pointers (unique and shared).
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::SmartPtr SP>
    std::string &serialize(SP const &elt, std::string &str) const {
        if (elt != nullptr) {
            if constexpr (serializer::concepts::Serializable<
                typename SP::element_type>) {
                return str.append(elt->serialize());
            } else {
                return serialize<typename SP::element_type>(*elt, str);
            }
        } else {
            return str.append("nullptr");
        }
    }

    /// @brief Deserialize function for smart pointers (unique and shared). A
    ///        pointer is allocated if required.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::SmartPtr SP>
    SP deserialize(std::string_view &str, SP &) {
        if (str.starts_with("nullptr")) {
            str = str.substr(7);
            return nullptr;
        }
        SP t;
        if constexpr (serializer::mtf::is_shared_v<SP>) {
            t = std::make_shared<typename SP::element_type>();
        } else if constexpr (serializer::mtf::is_unique_v<SP>) {
            t = std::make_unique<typename SP::element_type>();
        }
        if constexpr (serializer::concepts::Deserializable<
                          typename SP::element_type>) {
            t->deserialize(str);
        } else {
            *t = deserialize(str, *t);
        }
        return t;
    }

    /* tuples *****************************************************************/

    /// @brief Helper function used to serialize tuples.
    /// @param tuple Tuple to serialize
    /// @param str String that will contain the result.
    template <class T, size_t... Idx>
    std::string &serializeTuple(T const &tuple, std::string &str,
                                std::index_sequence<Idx...>) const {
        ([&] { serialize(std::get<Idx>(tuple), str); }(), ...);
        return str;
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::TupleLike T>
    requires(!serializer::concepts::Array<T>)
    std::string &serialize(T const &tuple, std::string &str) const {
        return serializeTuple(tuple, str,
                              std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /// @brief Helper function for deserializing tuples.
    /// @param str String that contains the data.
    template <class T, size_t... Idx>
    T deserializeTuple(std::string_view &str, std::index_sequence<Idx...>) {
        T tuple;
        (
            [&] {
                std::get<Idx>(tuple) = deserialize(str, std::get<Idx>(tuple));
            }(),
            ...);
        return tuple;
    }

    /// @brief Deserialize function tuples (std::tuple and std::pair).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::TupleLike T>
        requires(!serializer::concepts::Array<T>)
    T deserialize(std::string_view &str, T &) {
        return deserializeTuple<serializer::mtf::remove_const_tuple_t<T>>(
            str, std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Enum T>
    std::string &serialize(T const &elt, std::string &str) const {
        auto value = (std::underlying_type_t<T>)elt;
        return str.append(reinterpret_cast<const char *>(&value),
                          sizeof(value));
    }

    /// @brief Deserialize function for enum types. The data is stored using the
    ///        underlying type.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Enum T>
    T deserialize(std::string_view &str, T &) {
        using Type = std::underlying_type_t<T>;
        Type out = *reinterpret_cast<const Type *>(str.data());
        str = str.substr(sizeof(Type));
        return (T)out;
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    std::string &serialize(std::string const &elt, std::string &str) const {
        auto size = elt.size() + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        return str.append(elt);
    }

    /// @brief Deserialize function for strings.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::String T>
    std::string deserialize(std::string_view &str, T &) {
        using size_type = typename T::size_type;
        size_type size = deserialize(str, size) - 1;
        std::string t = std::string(str.substr(0, size));
        str = str.substr(size);
        return t;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::NonStringIterable T>
    std::string &serialize(T const &elts, std::string &str) const {
        auto size = std::size(elts) + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        for (auto elt : elts) {
            str = serialize(elt, str);
        }
        return str;
    }

    /// @brief Deserialize function for containers. They must be iterable and
    ///        insertable (implements insert, push_back methods or usable with
    ///        std::insert).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::NonStringIterable T>
    T deserialize(std::string_view &str, T &) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType = serializer::mtf::iter_value_t<T>;
        T result;
        size_t idx = 0;
        size_type size = deserialize(str, size) - 1;
        for (size_t i = 0; i < size; ++i) {
            ValueType value = deserialize(str, value);
            if constexpr (serializer::concepts::Insertable<T, ValueType> ||
                          serializer::concepts::PushBackable<T, ValueType>) {
                serializer::tools::insert(result, value);
            } else {
                serializer::tools::insert(result, value, idx++);
            }
        }
        return result;
    }
};

} // namespace serializer

#endif
