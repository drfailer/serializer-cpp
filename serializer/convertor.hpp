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
/*                          default convertor class                           */
/******************************************************************************/

namespace serializer {

template <typename T> struct Convert {
    virtual std::string &serialize_(T const &, std::string &) const = 0;
    virtual T deserialize_(std::string_view &, T &) = 0;
};

template <typename... AdditionalTypes>
struct Convertor : public Convert<AdditionalTypes>... {

    /* no automatic serialization types (custom convertor) ********************/

    template <serializer::concepts::NonSerializable T>
    std::string &serialize(T const &elt, std::string str) const {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            return this->serialize_(elt, str);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<T>();
        }
    }

    template <serializer::concepts::NonDeserializable T>
    T deserialize(std::string_view &str, T &elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            return this->deserialize_(str, elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<T>();
        }
    }

    /* serializable ***********************************************************/

    template <serializer::concepts::Serializable T>
    std::string &serialize(T const &elt, std::string &str) const {
        return str.append(elt.serialize());
    }

    template <serializer::concepts::Deserializable T>
    std::remove_reference_t<T> deserialize(std::string_view &str, T &) {
        std::remove_reference_t<T> t;
        t.deserialize(str);
        return t;
    }

    /* fundamental types ******************************************************/

    template <serializer::concepts::Fundamental T>
    std::string &serialize(T const &elt, std::string &str) const {
        return str.append(reinterpret_cast<const char *>(&elt), sizeof(T));
    }

    template <serializer::concepts::Fundamental T>
    T deserialize(std::string_view &str, T &) {
        T t = *reinterpret_cast<const T *>(str.data());
        str = str.substr(sizeof(T));
        return t;
    }

    /* pointers ***************************************************************/

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

    template <class T, size_t... Idx>
    std::string &serializeTuple(T const &tuple, std::string &str,
                                std::index_sequence<Idx...>) const {
        ([&] { serialize(std::get<Idx>(tuple), str); }(), ...);
        return str;
    }

    template <serializer::concepts::TupleLike T>
    requires(!serializer::concepts::Array<T>)
    std::string &serialize(T const &tuple, std::string &str) const {
        return serializeTuple(tuple, str,
                              std::make_index_sequence<std::tuple_size_v<T>>());
    }

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

    template <serializer::concepts::TupleLike T>
        requires(!serializer::concepts::Array<T>)
    T deserialize(std::string_view &str, T &) {
        return deserializeTuple<serializer::mtf::remove_const_tuple_t<T>>(
            str, std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /* enums ******************************************************************/

    template <serializer::concepts::Enum T>
    std::string &serialize(T const &elt, std::string &str) const {
        auto value = (std::underlying_type_t<T>)elt;
        return str.append(reinterpret_cast<const char *>(&value),
                          sizeof(value));
    }

    template <serializer::concepts::Enum T>
    T deserialize(std::string_view &str, T &) {
        using Type = std::underlying_type_t<T>;
        Type out = *reinterpret_cast<const Type *>(str.data());
        str = str.substr(sizeof(Type));
        return (T)out;
    }

    /* strings ****************************************************************/

    std::string &serialize(std::string const &elt, std::string &str) const {
        auto size = elt.size() + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        return str.append(elt);
    }

    template <serializer::concepts::String T>
    std::string deserialize(std::string_view &str, T &) {
        using size_type = typename T::size_type;
        size_type size = deserialize(str, size) - 1;
        std::string t = std::string(str.substr(0, size));
        str = str.substr(size);
        return t;
    }

    /* iterable containers ****************************************************/

    template <serializer::concepts::NonStringIterable T>
    std::string &serialize(T const &elts, std::string &str) const {
        auto size = std::size(elts) + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        for (auto elt : elts) {
            str = serialize(elt, str);
        }
        return str;
    }

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
                serializer::utility::insert(result, value);
            } else {
                serializer::utility::insert(result, value, idx++);
            }
        }
        return result;
    }
};

} // namespace serializer

#endif
