#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "concepts.hpp"
#include "metafunctions.hpp"
#include "parser.hpp"
#include "serializer/exceptions.hpp"
#include "utils.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

/******************************************************************************/
/*                      default convertor implementation                      */
/******************************************************************************/

/* I'm sorry for this :,(
 *
 * We want the user to be able to define it's own convertor to implement missing
 * convertion functions. The ideal solution would use inheritance but this
 * cannot work as the Convertor member functions cannot call the functions of
 * the derived class. So the first time the custom convertor calls one of the
 * functions in the default one, we cannot use custom functions anymore as they
 * are not visible for the default convertor.
 *
 * We could find a way for the default convertor and the custom one to know each
 * other and allow function calls between theme. However, the solution would be
 * even more complex and it would be really difficult to make it easy to use.
 *
 * This macro looks terrible and it makes the library annoying to maintain but
 * this is the best solution I have for know.
 */
#define CONVERTOR                                                              \
    /* deserialize ********************************************************/   \
                                                                               \
    template <serializer::concepts::Deserializable T>                          \
    static std::remove_reference_t<T> deserialize(const std::string &str) {    \
        std::remove_reference_t<T> t;                                          \
        t.deserialize(str);                                                    \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Fundamental T>                             \
    static T deserialize(const std::string &str) {                             \
        T t;                                                                   \
        std::istringstream(str) >> t;                                          \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::ConcretePtr T>                             \
    static std::remove_reference_t<T> deserialize(const std::string &str) {    \
        if (str == "nullptr") {                                                \
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
    static SP deserialize(const std::string &str) {                            \
        if (str == "nullptr") {                                                \
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
    static T deserializeTuple(const std::string &str,                          \
                              std::index_sequence<Idx...>) {                   \
        T tuple;                                                               \
        std::pair<std::string, std::string> content = std::make_pair("", str); \
        (                                                                      \
            [&] {                                                              \
                content = serializer::parser::parseTuple(content.second);      \
                std::get<Idx>(tuple) =                                         \
                    deserialize<std::tuple_element_t<Idx, T>>(content.first);  \
            }(),                                                               \
            ...);                                                              \
        return tuple;                                                          \
    }                                                                          \
                                                                               \
    template <serializer::concepts::TupleLike T>                               \
        requires(!serializer::concepts::Array<T>)                              \
    static T deserialize(const std::string &str) {                             \
        return deserializeTuple<serializer::mtf::remove_const_tuple_t<T>>(     \
            str, std::make_index_sequence<std::tuple_size_v<T>>());            \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Enum T>                                    \
    static T deserialize(const std::string &str) {                             \
        std::istringstream iss(str);                                           \
        std::underlying_type_t<T> out;                                         \
        iss >> out;                                                            \
        return (T)out;                                                         \
    }                                                                          \
                                                                               \
    template <serializer::concepts::String T>                                  \
    static std::string deserialize(const std::string &str) {                   \
        std::string t =                                                        \
            serializer::parser::unescapeStr(str.substr(1, str.size() - 2));    \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <serializer::concepts::NonStringIterable T>                       \
    static T deserialize(const std::string &str) {                             \
        T result;                                                              \
        std::size_t valueStart = 2;                                            \
        std::size_t valueEnd;                                                  \
        size_t idx = 0;                                                        \
        using valueType = serializer::mtf::iter_value_t<T>;                    \
                                                                               \
        while (valueStart < str.size()) {                                      \
            valueEnd = serializer::parser::findEndValueIndex(str, valueStart); \
            auto value = deserialize<valueType>(                               \
                str.substr(valueStart, valueEnd - valueStart));                \
            if constexpr (serializer::concepts::Insertable<T, valueType> ||    \
                          serializer::concepts::PushBackable<T, valueType>) {  \
                serializer::utility::insert(result, value);                    \
            } else {                                                           \
                serializer::utility::insert(result, value, idx++);             \
            }                                                                  \
            valueStart = valueEnd + 2; /* value1, value2 */                    \
        }                                                                      \
                                                                               \
        return result;                                                         \
    }                                                                          \
                                                                               \
    /* serialize ***********************************************************/  \
                                                                               \
    template <serializer::concepts::Serializable T>                            \
    static std::string serialize(T &elt) {                                     \
        return elt.serialize();                                                \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Fundamental T>                             \
    static std::string serialize(T &elt) {                                     \
        std::ostringstream oss;                                                \
        oss << elt;                                                            \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Pointer T>                                 \
    static std::string serialize(T elt) {                                      \
        if (elt != nullptr) {                                                  \
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {      \
                return elt->serialize();                                       \
            } else {                                                           \
                return serialize<std::remove_pointer_t<T>>(*elt);              \
            }                                                                  \
        } else {                                                               \
            return "nullptr";                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <serializer::concepts::SmartPtr SP>                               \
    static std::string serialize(SP &elt) {                                    \
        if (elt != nullptr) {                                                  \
            if constexpr (serializer::concepts::Serializable<                  \
                              typename SP::element_type>) {                    \
                return elt->serialize();                                       \
            } else {                                                           \
                return serialize<typename SP::element_type>(*elt);             \
            }                                                                  \
        } else {                                                               \
            return "nullptr";                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <class T, size_t... Idx>                                          \
    static std::string serializeTuple(const T &tuple,                          \
                                      std::index_sequence<Idx...>) {           \
        std::ostringstream oss;                                                \
        oss << "{ ";                                                           \
        (oss << ... << (serialize(std::get<Idx>(tuple)) + ", "));              \
        oss << " }";                                                           \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <serializer::concepts::TupleLike T>                               \
        requires(!serializer::concepts::Array<T>)                              \
    static std::string serialize(const T &tuple) {                             \
        return serializeTuple(                                                 \
            tuple, std::make_index_sequence<std::tuple_size_v<T>>());          \
    }                                                                          \
                                                                               \
    template <serializer::concepts::Enum T>                                    \
    static std::string serialize(const T &elt) {                               \
        std::ostringstream oss;                                                \
        std::underlying_type_t<T> v = (std::underlying_type_t<T>)elt;          \
        oss << v;                                                              \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    static std::string serialize(const std::string &elt) {                     \
        std::ostringstream oss;                                                \
        oss << "\"" << serializer::parser::escapeStr(elt) << "\"";             \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <serializer::concepts::NonStringIterable T>                       \
    static std::string serialize(const T &elts) {                              \
        std::ostringstream oss;                                                \
        auto it = elts.begin();                                                \
                                                                               \
        if (it != elts.end()) {                                                \
            oss << "[ " << serialize(*it++);                                   \
            for (; it != elts.end(); it++) {                                   \
                oss << ", " << serialize(*it);                                 \
            }                                                                  \
            oss << " ]";                                                       \
        }                                                                      \
        return oss.str();                                                      \
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
    static T deserialize(const std::string &) {
        throw serializer::exceptions::UnsupportedTypeError<T>();
    }

    CONVERTOR;
};

}

#endif
