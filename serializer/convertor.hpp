#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "concepts.hpp"
#include "metafunctions.hpp"
#include "parser.hpp"
#include "serializer/exceptions.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/******************************************************************************/
/*                      default convertor implementation                      */
/******************************************************************************/

/* I'm sorry for this :,(
 *
 * We want the user to be able to define its own convertor to implement missing
 * conversion functions. The ideal solution would use inheritance but this
 * cannot work as the Convertor member functions cannot call the functions of
 * the derived class. So the first time the custom convertor calls one of the
 * functions in the default one, we cannot use custom functions anymore as they
 * are not visible for the default convertor.
 *
 * We could find a way for the default convertor and the custom one to know each
 * other and allow function calls between theme. However, the solution would be
 * even more complex, and it would be really difficult to make it easy to use.
 *
 * This macro looks terrible, and it makes the library annoying to maintain but
 * this is the best solution I have for know.
 */
#define CONVERTOR                                                              \
    /* deserialize ********************************************************/   \
                                                                               \
    template <serializer::concepts::Deserializable T>                          \
    static std::remove_reference_t<T> deserialize(std::string_view &str) {     \
        std::remove_reference_t<T> t;                                          \
        t.deserialize(str);                                                    \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Fundamental T>                             \
    static T deserialize(std::string_view &str) {                              \
        T t = *reinterpret_cast<const T *>(str.data());                        \
        str = str.substr(sizeof(T));                                           \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::ConcretePtr T>                             \
    static std::remove_reference_t<T> deserialize(std::string_view &str) {     \
        if (str.starts_with("nullptr")) {                                      \
            str = str.substr(7);                                               \
            return nullptr;                                                    \
        }                                                                      \
        using Type =                                                           \
            typename std::remove_pointer_t<std::remove_reference_t<T>>;        \
        Type *t = new Type();                                                  \
                                                                               \
        if constexpr (std::is_fundamental_v<Type>) {                           \
            *t = deserialize<Type>(str);                                       \
        } else {                                                               \
            t->deserialize(str);                                               \
        }                                                                      \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::SmartPtr SP>                               \
    static SP deserialize(std::string_view &str) {                             \
        if (str.starts_with("nullptr")) {                                      \
            str = str.substr(7);                                               \
            return nullptr;                                                    \
        }                                                                      \
        SP t;                                                                  \
        if constexpr (serializer::mtf::is_shared_v<SP>) {                      \
            t = std::make_shared<typename SP::element_type>();                 \
        } else if constexpr (serializer::mtf::is_unique_v<SP>) {               \
            t = std::make_unique<typename SP::element_type>();                 \
        }                                                                      \
                                                                               \
        if constexpr (serializer::concepts::Deserializable<                    \
                          typename SP::element_type>) {                        \
            t->deserialize(str);                                               \
        } else {                                                               \
            *t = deserialize<typename SP::element_type>(str);                  \
        }                                                                      \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <class T, size_t... Idx>                                          \
    static T deserializeTuple(std::string_view &str,                           \
                              std::index_sequence<Idx...>) {                   \
        T tuple;                                                               \
        (                                                                      \
            [&] {                                                              \
                std::get<Idx>(tuple) =                                         \
                    deserialize<typename std::tuple_element<Idx, T>::type>(    \
                        str);                                                  \
            }(),                                                               \
            ...);                                                              \
        return tuple;                                                          \
    }                                                                          \
                                                                               \
    template <serializer::concepts::TupleLike T>                               \
        requires(!serializer::concepts::Array<T>)                              \
    static T deserialize(std::string_view &str) {                              \
        return deserializeTuple<serializer::mtf::remove_const_tuple_t<T>>(     \
            str, std::make_index_sequence<std::tuple_size_v<T>>());            \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Enum T>                                    \
    static T deserialize(std::string_view &str) {                              \
        using Type = std::underlying_type_t<T>;                                \
        Type out = *reinterpret_cast<const Type *>(str.data());                \
        str = str.substr(sizeof(Type));                                        \
        return (T)out;                                                         \
    }                                                                          \
                                                                               \
    template <serializer::concepts::String T>                                  \
    static std::string deserialize(std::string_view &str) {                    \
        using size_type = typename T::size_type;                               \
        size_type size = deserialize<size_type>(str) - 1;                      \
        std::string t = std::string(str.substr(0, size));                      \
        str = str.substr(size);                                                \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::NonStringIterable T>                       \
    static T deserialize(std::string_view &str) {                              \
        using size_type = decltype(std::size(std::declval<T>()));              \
        using ValueType = serializer::mtf::iter_value_t<T>;                    \
        T result;                                                              \
        size_t idx = 0;                                                        \
        size_type size = deserialize<size_type>(str) - 1;                      \
                                                                               \
        for (size_t i = 0; i < size; ++i) {                                    \
            ValueType value = deserialize<ValueType>(str);                     \
            if constexpr (serializer::concepts::Insertable<T, ValueType> ||    \
                          serializer::concepts::PushBackable<T, ValueType>) {  \
                serializer::utility::insert(result, value);                    \
            } else {                                                           \
                serializer::utility::insert(result, value, idx++);             \
            }                                                                  \
        }                                                                      \
                                                                               \
        return result;                                                         \
    }                                                                          \
                                                                               \
    /* serialize ***********************************************************/  \
                                                                               \
    template <serializer::concepts::Serializable T>                            \
    static std::string serialize(T &elt, std::string &str) {                   \
        /* TODO: optimize this */                                              \
        return str.append(elt.serialize());                                    \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Fundamental T>                             \
    static std::string serialize(T &elt, std::string &str) {                   \
        return str.append(reinterpret_cast<const char *>(&elt), sizeof(T));    \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Pointer T>                                 \
    static std::string serialize(T elt, std::string &str) {                    \
        if (elt != nullptr) {                                                  \
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {      \
                str.append(elt->serialize());                                  \
                return str;                                                    \
            } else {                                                           \
                /* ptr mode: address or content ? */                           \
                return serialize<std::remove_pointer_t<T>>(*elt, str);         \
            }                                                                  \
        } else {                                                               \
            return str.append("nullptr");                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <serializer::concepts::SmartPtr SP>                               \
    static std::string serialize(SP &elt, std::string &str) {                  \
        if (elt != nullptr) {                                                  \
            if constexpr (serializer::concepts::Serializable<                  \
                              typename SP::element_type>) {                    \
                return str.append(elt->serialize());                           \
            } else {                                                           \
                return serialize<typename SP::element_type>(*elt, str);        \
            }                                                                  \
        } else {                                                               \
            return str.append("nullptr");                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <class T, size_t... Idx>                                          \
    static std::string serializeTuple(const T &tuple, std::string &str,        \
                                      std::index_sequence<Idx...>) {           \
        ([&] { serialize(std::get<Idx>(tuple), str); }(), ...);                \
        return str;                                                            \
    }                                                                          \
                                                                               \
    template <serializer::concepts::TupleLike T>                               \
        requires(!serializer::concepts::Array<T>)                              \
    static std::string serialize(const T &tuple, std::string &str) {           \
        return serializeTuple(                                                 \
            tuple, str, std::make_index_sequence<std::tuple_size_v<T>>());     \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Enum T>                                    \
    static std::string serialize(const T &elt, std::string &str) {             \
        std::underlying_type_t<T> value = (std::underlying_type_t<T>)elt;      \
        return str.append(reinterpret_cast<const char *>(&value),              \
                          sizeof(value));                                      \
    }                                                                          \
                                                                               \
    static std::string serialize(const std::string &elt, std::string &str) {   \
        auto size = elt.size() + 1;                                            \
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));       \
        return str.append(elt);                                                \
    }                                                                          \
                                                                               \
    template <serializer::concepts::NonStringIterable T>                       \
    static std::string serialize(const T &elts, std::string &str) {            \
        auto size = std::size(elts) + 1;                                       \
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));       \
                                                                               \
        for (auto elt : elts) {                                                \
            serialize(elt, str);                                               \
        }                                                                      \
        return str;                                                            \
    }

/******************************************************************************/
/*                          default convertor class                           */
/******************************************************************************/

namespace serializer {

struct Convertor {
    template <serializer::concepts::NonSerializable T>
    static std::string serialize(const T &) {
        throw serializer::exceptions::UnsupportedTypeError<T>();
    }

    template <serializer::concepts::NonDeserializable T>
    static T deserialize(std::string_view &) {
        throw serializer::exceptions::UnsupportedTypeError<T>();
    }

    CONVERTOR;
};

} // namespace serializer

#endif
