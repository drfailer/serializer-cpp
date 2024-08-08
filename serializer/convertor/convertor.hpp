#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "../exceptions/unsupported_type.hpp"
#include "../tools/c_struct.hpp"
#include "../tools/concepts.hpp"
#include "../tools/dynamic_array.hpp"
#include "../tools/metafunctions.hpp"
#include "../tools/tools.hpp"
#include "convert.hpp"
#include "serializer/tools/ml_arg_type.hpp"
#include <algorithm>
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
    template <serializer::tools::concepts::NonSerializable T>
        requires(!tools::mtf::is_c_struct_v<T>)
    std::string &serialize_(T const &elt, std::string str) const {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            return static_cast<const Convert<T> *>(this)->serialize(elt, str);
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
    template <serializer::tools::concepts::NonDeserializable T>
        requires(!tools::mtf::is_c_struct_v<T>)
    T deserialize_(std::string_view &str, T &elt) {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            return static_cast<Convert<T> *>(this)->deserialize(str, elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<T>();
        }
    }

    /* serializable ***********************************************************/

    /// @brief Serialize function for the serializable types (they have a
    ///        serialize method).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Serializable T>
    std::string &serialize_(T const &elt, std::string &str) const {
        return str.append(elt.serialize());
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Deserializable T>
    T &deserialize_(std::string_view &str, T &elt) {
        elt.deserialize(str);
        return elt;
    }

    /* fundamental types ******************************************************/

    /// @brief Serialize function for the fundamental types.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Fundamental T>
    std::string &serialize_(T const &elt, std::string &str) const {
        return str.append(reinterpret_cast<const char *>(&elt), sizeof(T));
    }

    /// @brief Deserialize function for the deserializable fundamental types.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Fundamental T>
    T deserialize_(std::string_view &str, T &) {
        T t = *reinterpret_cast<const T *>(str.data());
        str = str.substr(sizeof(T));
        return t;
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Pointer T>
    std::string &serialize_(T const &elt, std::string &str) const {
        if (elt != nullptr) {
            str.append("v");
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {
                return elt->serialize(str);
            } else {
                return serialize_<std::remove_pointer_t<T>>(*elt, str);
            }
        } else {
            return str.append("n");
        }
    }

    /// @brief Deserialize function for the pointer types. If the pointer is not
    ///        null, a dynamic allocation is performed before deserializing the
    ///        result. This memory should be handled by the user.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::ConcretePtr T>
    T deserialize_(std::string_view &str, T &elt) {
        bool ptrValid = str[0] == 'v';
        str = str.substr(1);

        if (!ptrValid) {
            return nullptr;
        }
        static_assert(std::is_default_constructible_v<std::remove_pointer_t<T>>,
                      "The pointer types should be default constructible.");
        using Type = typename std::remove_pointer_t<std::remove_reference_t<T>>;
        if (elt == nullptr) {
            elt = new Type();
        }
        if constexpr (std::is_fundamental_v<Type>) {
            *elt = deserialize_(str, *elt);
        } else {
            elt->deserialize(str);
        }
        return elt;
    }

    /* smart pointers *********************************************************/

    /// @brief Serialize function for smart pointers (unique and shared).
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::SmartPtr SP>
    std::string &serialize_(SP const &elt, std::string &str) const {
        if (elt != nullptr) {
            str.append("v");
            if constexpr (serializer::tools::concepts::Serializable<
                              typename SP::element_type>) {
                return elt->serialize(str);
            } else {
                return serialize_<typename SP::element_type>(*elt, str);
            }
        } else {
            return str.append("n");
        }
    }

    /// @brief Deserialize function for smart pointers (unique and shared). A
    ///        pointer is allocated if required.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::ConcreteSmartPtr SP>
    SP deserialize_(std::string_view &str, SP &) {
        bool ptrValid = str[0] == 'v';
        str = str.substr(1);

        if (!ptrValid) {
            return nullptr;
        }
        SP t;
        static_assert(
            std::is_default_constructible_v<std::remove_pointer_t<SP>>,
            "The pointer types should be default constructible.");
        if constexpr (serializer::tools::mtf::is_shared_v<SP>) {
            t = std::make_shared<typename SP::element_type>();
        } else if constexpr (serializer::tools::mtf::is_unique_v<SP>) {
            t = std::make_unique<typename SP::element_type>();
        }
        if constexpr (serializer::tools::concepts::Deserializable<
                          typename SP::element_type>) {
            t->deserialize(str);
        } else {
            *t = deserialize_(str, *t);
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
        ([&] { serialize_(std::get<Idx>(tuple), str); }(), ...);
        return str;
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::TupleLike T>
        requires(!serializer::tools::concepts::Array<T>)
    std::string &serialize_(T const &tuple, std::string &str) const {
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
                using Type = decltype(std::get<Idx>(tuple));
                if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                  Type> ||
                              tools::concepts::Serializable<Type>) {
                    deserialize_(str, std::get<Idx>(tuple));
                } else {
                    std::get<Idx>(tuple) =
                        deserialize_(str, std::get<Idx>(tuple));
                }
            }(),
            ...);
        return tuple;
    }

    /// @brief Deserialize function tuples (std::tuple and std::pair).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::TupleLike T>
        requires(!serializer::tools::concepts::Array<T>)
    T deserialize_(std::string_view &str, T &) {
        return deserializeTuple<
            serializer::tools::mtf::remove_const_tuple_t<T>>(
            str, std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Enum T>
    std::string &serialize_(T const &elt, std::string &str) const {
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
    template <serializer::tools::concepts::Enum T>
    T deserialize_(std::string_view &str, T &) {
        using Type = std::underlying_type_t<T>;
        Type out = *reinterpret_cast<const Type *>(str.data());
        str = str.substr(sizeof(Type));
        return (T)out;
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    std::string &serialize_(std::string const &elt, std::string &str) const {
        auto size = elt.size() + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        return str.append(elt);
    }

    /// @brief Deserialize function for strings.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::String T>
    std::string deserialize_(std::string_view &str, T &) {
        using size_type = typename T::size_type;
        size_type size = deserialize_(str, size) - 1;
        std::string t = std::string(str.substr(0, size));
        str = str.substr(size);
        return t;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::NonStringIterable T>
    std::string &serialize_(T const &elts, std::string &str) const {
        auto size = std::size(elts) + 1;
        str.append(reinterpret_cast<const char *>(&size), sizeof(size));
        for (auto const &elt : elts) {
            str = serialize_(elt, str);
        }
        return str;
    }

    /// @brief Deserialize function for containers. They must be iterable and
    ///        insertable (implements insert, add methods or usable with
    ///        std::insert).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::NonStringIterable T>
    T deserialize_(std::string_view &str, T &) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType = serializer::tools::mtf::iter_value_t<T>;
        T result;
        size_t idx = 0;
        size_type size = deserialize_(str, size) - 1;
        for (size_t i = 0; i < size; ++i) {
          if constexpr (tools::mtf::not_assigned_on_deserialization_v<ValueType> ||
            tools::concepts::Deserializable<ValueType>) {
            ValueType value;
            deserialize_(str, value);
            if constexpr (serializer::tools::concepts::Insertable<T,
                ValueType> ||
                serializer::tools::concepts::PushBackable<
                T, ValueType>) {
              serializer::tools::insert(result, std::move(value));
            } else {
              serializer::tools::insert(result, std::move(value), idx++);
            }
          } else {
            ValueType value = deserialize_(str, value);
            if constexpr (serializer::tools::concepts::Insertable<T,
                ValueType> ||
                serializer::tools::concepts::PushBackable<
                T, ValueType>) {
              serializer::tools::insert(result, std::move(value));
            } else {
              serializer::tools::insert(result, std::move(value), idx++);
            }
          }
        }
        return result;
    }

    /* static array ***********************************************************/

    /// @brief Serialize function for static arrays.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::StaticArray T>
    std::string &serialize_(T const &elt, std::string &str) const {
        for (size_t i = 0; i < std::extent_v<T>; ++i) {
            serialize_(elt[i], str);
        }
        return str;
    }

    /// @brief Deserialize function for static arrays.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <serializer::tools::concepts::StaticArray T>
    void deserialize_(std::string_view &str, T &elt) {
        using InnerType = std::remove_extent_t<T>;
        for (size_t i = 0; i < std::extent_v<T>; ++i) {
            if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                              InnerType> ||
                          tools::concepts::Deserializable<InnerType>) {
                deserialize_(str, elt[i]);
            } else {
                elt[i] = deserialize_(str, elt[i]);
            }
        }
    }

    /* dynamic array **********************************************************/

    /// @brief Serialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <tools::concepts::Pointer T, typename DT, typename... DTs>
    std::string &serialize_(tools::DynamicArray<T, DT, DTs...> const &elt,
                            std::string &str) const {
        using ST = std::remove_pointer_t<T>;
        if (elt.mem == nullptr) {
            return str.append("n");
        }
        str.append("v");

        if constexpr (std::is_pointer_v<ST>) {
            for (size_t i = 0; i < std::get<0>(elt.dimensions); ++i) {
                str = serialize_(
                    tools::DynamicArray<ST, DTs...>(
                        elt.mem[i], tools::tuplePopFront(elt.dimensions)),
                    str);
            }
        } else if constexpr (std::is_fundamental_v<ST>) {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            str.append(reinterpret_cast<char *>(elt.mem), size * sizeof(ST));
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            for (size_t i = 0; i < size; ++i) {
                str = serialize_(elt.mem[i], str);
            }
        }
        return str;
    }

    /// @brief Deserialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    ///        Note: memory is allocated if required. If the pointer is not set
    ///        correctly, this function may segfault. To avoid pointer
    ///        management, use the containers of the standard library instead.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <tools::concepts::Pointer T, typename DT, typename... DTs>
    void deserialize_(std::string_view &str,
                      tools::DynamicArray<T, DT, DTs...> &elt) {
        using ST = std::remove_pointer_t<T>;
        bool ptrValid = str[0] == 'v';
        str = str.substr(1);

        if (!ptrValid) {
            elt.mem = nullptr;
            return;
        }
        if (elt.mem == nullptr) {
            elt.mem = new ST[std::get<0>(elt.dimensions)]();
        }

        if constexpr (std::is_pointer_v<ST>) {
            for (size_t i = 0; i < std::get<0>(elt.dimensions); ++i) {
                tools::DynamicArray<ST, DTs...> subArray(
                    elt.mem[i], tools::tuplePopFront(elt.dimensions));
                deserialize_(str, subArray);
            }
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            for (size_t i = 0; i < size; ++i) {
                if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                  ST> ||
                              tools::concepts::Deserializable<ST>) {
                    deserialize_(str, elt.mem[i]);
                } else {
                    elt.mem[i] = deserialize_(str, elt.mem[i]);
                }
            }
        }
    }

    /* C Structs **************************************************************/

    template <typename T>
    std::string &serialize_(tools::CStruct<T> const &elt,
                            std::string &str) const {
        str.append(reinterpret_cast<const char *>(&elt.element),
                   sizeof(elt.element));
        return str;
    }

    template <typename T>
    void deserialize_(std::string_view &str, tools::CStruct<T> &elt) {
        elt.element = *reinterpret_cast<const T *>(str.data());
        str = str.substr(sizeof(elt.element));
    }
};

} // namespace serializer

#endif
