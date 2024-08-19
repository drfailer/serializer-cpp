#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "../exceptions/unsupported_type.hpp"
#include "../tools/c_struct.hpp"
#include "../tools/concepts.hpp"
#include "../tools/dynamic_array.hpp"
#include "../tools/metafunctions.hpp"
#include "../tools/ml_arg_type.hpp"
#include "../tools/tools.hpp"
#include "convert.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <iostream>
#include <span>
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
template <typename MemT, typename... AdditionalTypes>
struct Convertor : public Convert<AdditionalTypes>... {
    using mem_type = MemT;
    mem_type &mem;

    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    using view_type = std::span<byte_type>;

    size_t spos = 0;
    size_t dpos = 0;
    size_t size = 0;
    size_t capacity = 0;

    constexpr Convertor(MemT &mem)
        : mem(mem), size(mem.size()), capacity(mem.capacity()) {}

    constexpr void append(const byte_type *bytes, size_t nb_bytes) {
        if ((spos + nb_bytes) >= capacity) {
            if (capacity == 0) [[unlikely]] {
                capacity = nb_bytes;
            } else {
                capacity *= 2;
            }
            mem.resize(capacity);
        }
        std::memcpy(mem.data() + spos, bytes, nb_bytes);
        spos += nb_bytes;
        mem.resize(spos);
    }

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
    constexpr void serialize_(T const &elt) {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            static_cast<const Convert<T> *>(this)->serialize(elt, mem);
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
    constexpr T deserialize_(T &elt) {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            // TODO: rework convert functions
            return static_cast<Convert<T> *>(this)->deserialize(elt);
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
    constexpr void serialize_(T const &elt) {
        elt.serialize(mem);
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Deserializable T>
    constexpr T &deserialize_(T &elt) {
        // TODO: create a new deserialze function
        /* elt.deserialize(view); */
        return elt;
    }

    /* fundamental types ******************************************************/

    /// @brief Serialize function for the fundamental types.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Fundamental T>
    constexpr void serialize_(T const &elt) {
        append(std::bit_cast<const byte_type *>(&elt), sizeof(T));
    }

    /// @brief Deserialize function for the deserializable fundamental types.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Fundamental T>
    constexpr T deserialize_(T &) {
        T t = *std::bit_cast<const T *>(mem.data() + dpos);
        dpos += sizeof(T);
        return t;
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Pointer T>
    constexpr void serialize_(T const &elt) {
        if (elt != nullptr) {
            append('v', 1);
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {
                elt->serialize(mem);
            } else {
                serialize_<std::remove_pointer_t<T>>(*elt);
            }
        } else {
            append('n', 1);
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
    constexpr T deserialize_(T &elt) {
        bool ptrValid = mem[dpos++] == 'v';

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
            *elt = deserialize_(*elt);
        } else {
            // TODO: rework the deserialize function
            /* elt->deserialize(str); */
        }
        return elt;
    }

    /* smart pointers *********************************************************/

    /// @brief Serialize function for smart pointers (unique and shared).
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::SmartPtr SP>
    constexpr void serialize_(SP const &elt) {
        if (elt != nullptr) {
            mem[spos++] = 'v';
            if constexpr (serializer::tools::concepts::Serializable<
                              typename SP::element_type>) {
                elt->serialize(mem);
            } else {
                serialize_<typename SP::element_type>(*elt);
            }
        } else {
            mem[spos++] = 'n';
        }
    }

    /// @brief Deserialize function for smart pointers (unique and shared). A
    ///        pointer is allocated if required.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::ConcreteSmartPtr SP>
    constexpr SP deserialize_(SP &) {
        bool ptrValid = mem[dpos++] == 'v';

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
            // TODO: rework deserialize function
            /* t->deserialize(str); */
        } else {
            *t = deserialize_(*t);
        }
        return t;
    }

    /* tuples *****************************************************************/

    /// @brief Helper function used to serialize tuples.
    /// @param tuple Tuple to serialize
    /// @param str String that will contain the result.
    template <class T, size_t... Idx>
    constexpr void serializeTuple(T const &tuple, std::index_sequence<Idx...>) {
        ([&] { serialize_(std::get<Idx>(tuple)); }(), ...);
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::TupleLike T>
        requires(!serializer::tools::concepts::Array<T>)
    constexpr void serialize_(T const &tuple) {
        serializeTuple(tuple, mem,
                       std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /// @brief Helper function for deserializing tuples.
    /// @param str String that contains the data.
    template <class T, size_t... Idx>
    constexpr T deserializeTuple(std::index_sequence<Idx...>) {
        T tuple;
        (
            [&] {
                using Type = decltype(std::get<Idx>(tuple));
                if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                  Type> ||
                              tools::concepts::Serializable<Type>) {
                    deserialize_(std::get<Idx>(tuple));
                } else {
                    std::get<Idx>(tuple) = deserialize_(std::get<Idx>(tuple));
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
    constexpr T deserialize_(T &) {
        return deserializeTuple<
            serializer::tools::mtf::remove_const_tuple_t<T>>(
            std::make_index_sequence<std::tuple_size_v<T>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Enum T>
    constexpr void serialize_(T const &elt) {
        auto value = (std::underlying_type_t<T>)elt;
        append(std::bit_cast<const byte_type *>(&value), sizeof(value));
    }

    /// @brief Deserialize function for enum types. The data is stored using the
    ///        underlying type.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Enum T>
    constexpr T deserialize_(T &) {
        using Type = std::underlying_type_t<T>;
        Type out = *std::bit_cast<const Type *>(mem.data() + dpos);
        dpos += sizeof(Type);
        return (T)out;
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    constexpr void serialize_(std::string const &elt) {
        using size_type = typename std::string::size_type;
        size_type size = elt.size() + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));
        append(std::bit_cast<const byte_type *>(elt.data()), elt.size());
    }

    /// @brief Deserialize function for strings.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::String T>
    constexpr void deserialize_(T &str) {
        using size_type = typename T::size_type;
        size_type size = deserialize_(size) - 1;
        str.reserve(size);
        std::memcpy(str.data(), std::bit_cast<const char *>(mem.data() + dpos), size);
        str[size] = 0;
        dpos += size;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::NonStringIterable T>
    constexpr void serialize_(T const &elts) {
        auto size = std::size(elts) + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));
        for (auto const &elt : elts) {
            serialize_(elt);
        }
    }

    /// @brief Deserialize function for containers. They must be iterable and
    ///        insertable (implements insert, add methods or usable with
    ///        std::insert).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::NonStringIterable T>
    constexpr T deserialize_(T &) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType = serializer::tools::mtf::iter_value_t<T>;
        T result;
        size_t idx = 0;
        size_type size = deserialize_(size) - 1;
        auto insert = [&result, &idx](ValueType &&value) {
            if constexpr (serializer::tools::concepts::Insertable<T,
                                                                  ValueType> ||
                          serializer::tools::concepts::PushBackable<
                              T, ValueType>) {
                (void)idx;
                serializer::tools::insert(result, std::move(value));
            } else {
                serializer::tools::insert(result, std::move(value), idx++);
            }
        };

        for (size_t i = 0; i < size; ++i) {
            if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                              ValueType> ||
                          tools::concepts::Deserializable<ValueType>) {
                ValueType value;
                deserialize_(value);
                insert(std::move(value));
            } else {
                ValueType value = deserialize_(value);
                insert(std::move(value));
            }
        }
        return result;
    }

    /* static array ***********************************************************/

    /// @brief Serialize function for static arrays.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::StaticArray T>
    constexpr void serialize_(T const &elt) {
        for (size_t i = 0; i < std::extent_v<T>; ++i) {
            serialize_(elt[i]);
        }
    }

    /// @brief Deserialize function for static arrays.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <serializer::tools::concepts::StaticArray T>
    constexpr void deserialize_(T &elt) {
        using InnerType = std::remove_extent_t<T>;
        for (size_t i = 0; i < std::extent_v<T>; ++i) {
            if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                              InnerType> ||
                          tools::concepts::Deserializable<InnerType>) {
                deserialize_(elt[i]);
            } else {
                elt[i] = deserialize_(elt[i]);
            }
        }
    }

    /* dynamic array **********************************************************/

    /// @brief Serialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <tools::concepts::Pointer T, typename DT, typename... DTs>
    constexpr void serialize_(tools::DynamicArray<T, DT, DTs...> const &elt) {
        using ST = std::remove_pointer_t<T>;
        if (elt.mem == nullptr) {
            mem[spos++] = 'n';
            return;
        }
        mem[spos++] = 'v';

        if constexpr (std::is_pointer_v<ST>) {
            for (size_t i = 0; i < std::get<0>(elt.dimensions); ++i) {
                serialize_(tools::DynamicArray<ST, DTs...>(
                    elt.mem[i], tools::tuplePopFront(elt.dimensions)));
            }
        } else if constexpr (std::is_fundamental_v<ST>) {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            append(std::bit_cast<char *>(elt.mem), size * sizeof(ST));
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            for (size_t i = 0; i < size; ++i) {
                serialize_(elt.mem[i]);
            }
        }
    }

    /// @brief Deserialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    ///        Note: memory is allocated if required. If the pointer is not set
    ///        correctly, this function may segfault. To avoid pointer
    ///        management, use the containers of the standard library instead.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <tools::concepts::Pointer T, typename DT, typename... DTs>
    constexpr void deserialize_(tools::DynamicArray<T, DT, DTs...> &elt) {
        using ST = std::remove_pointer_t<T>;
        bool ptrValid = mem[dpos++] == 'v';

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
                deserialize_(subArray);
            }
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            for (size_t i = 0; i < size; ++i) {
                if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                  ST> ||
                              tools::concepts::Deserializable<ST>) {
                    deserialize_(elt.mem[i]);
                } else {
                    elt.mem[i] = deserialize_(elt.mem[i]);
                }
            }
        }
    }
};

} // namespace serializer

#endif
