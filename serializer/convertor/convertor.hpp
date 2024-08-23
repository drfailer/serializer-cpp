#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "../exceptions/unsupported_type.hpp"
#include "../meta/concepts.hpp"
#include "../meta/type_check.hpp"
#include "../meta/type_transform.hpp"
#include "../tools/dynamic_array.hpp"
#include "../tools/tools.hpp"
#include "convert.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <iostream>
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
struct Convertor : Convert<AdditionalTypes>... {
    MemT &mem;
    size_t pos = 0;

    using mem_type = MemT;
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;

    constexpr Convertor(MemT &mem, size_t pos = 0) : mem(mem), pos(pos) {}

    /* helper functions *******************************************************/

    inline constexpr size_t deserialize_size() {
        size_t size = *std::bit_cast<const size_t *>(mem.data() + pos);
        pos += sizeof(size);
        return size - 1;
    }

    inline constexpr void append(const byte_type *bytes, size_t nb_bytes) {
        if constexpr (mtf::is_vec_v<mtf::base_t<mem_type>>) {
            mem.append(pos, bytes, nb_bytes);
            pos += nb_bytes;
        } else {
            if (mem.size() < pos + nb_bytes) [[unlikely]] {
                if constexpr (concepts::Resizeable<mem_type>) {
                    mem.resize((mem.size() + nb_bytes) * 2);
                } else {
                    throw std::out_of_range(
                        "error: serialization array too small.");
                }
            }
            std::memcpy(mem.data() + pos, bytes, nb_bytes);
            pos += nb_bytes;
        }
    }

    inline constexpr void append(char chr) {
        append(std::bit_cast<const byte_type *>(&chr), 1);
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
    template <typename T>
        requires(mtf::contains_v<T, AdditionalTypes...>
            || (concepts::NonSerializable<T> && !mtf::is_dynamic_array_v<T>))
    inline constexpr void serialize_(T &&elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            static_cast<Convert<mtf::base_t<T>> *>(this)->serialize(elt);
        } else {
            static_assert(concepts::Serializable<T>);
            throw serializer::exceptions::UnsupportedTypeError<
                mtf::base_t<T>>();
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
    template <typename T>
        requires(mtf::contains_v<T, AdditionalTypes...>
            || (concepts::NonDeserializable<T> && !mtf::is_dynamic_array_v<T>))
    inline constexpr void deserialize_(T &&elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            // TODO: rework convert functions
            static_cast<Convert<mtf::base_t<T>> *>(this)->deserialize(elt);
        } else {
            static_assert(mtf::contains_v<T, AdditionalTypes...>);
            throw serializer::exceptions::UnsupportedTypeError<
                mtf::base_t<T>>();
        }
    }

    /* serializable ***********************************************************/

    /// @brief Serialize function for the serializable types (they have a
    ///        serialize method).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Serializable T>
      requires (!mtf::contains_v<T, AdditionalTypes...>)
    inline constexpr void serialize_(T &&elt) {
        pos = elt.serialize(mem, pos);
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Deserializable T>
      requires (!mtf::contains_v<T, AdditionalTypes...>)
    inline constexpr void deserialize_(T &&elt) {
        pos = elt.deserialize(mem, pos);
    }

    /* fundamental types ******************************************************/

    /// @brief Serialize function for the fundamental types.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Trivial T>
    inline constexpr void serialize_(T &&elt) {
        append(std::bit_cast<const byte_type *>(&elt), sizeof(elt));
    }

    /// @brief Deserialize function for the deserializable fundamental types.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Trivial T>
    inline constexpr void deserialize_(T &&elt) {
        elt = *std::bit_cast<const mtf::base_t<T> *>(mem.data() + pos);
        pos += sizeof(T);
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Pointer T>
      requires (!mtf::contains_v<T, AdditionalTypes...>)
    inline constexpr void serialize_(T &&elt) {
        if (elt == nullptr) {
            append('n');
            return;
        }
        append('v');
        if constexpr (requires { elt->serialize(mem, pos); }) {
            pos = elt->serialize(mem, pos);
        } else {
          std::cout << "not working" << std::endl;
            serialize_(*elt);
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
      requires (!mtf::contains_v<T, AdditionalTypes...>)
    inline constexpr void deserialize_(T &&elt) {
        bool ptrValid = mem[pos++] == 'v';

        if (!ptrValid) {
            elt = nullptr;
            return;
        }
        static_assert(std::is_default_constructible_v<
                          std::remove_pointer_t<std::remove_cvref_t<T>>>,
                      "The pointer types should be default constructible.");
        using Type = typename std::remove_pointer_t<std::remove_reference_t<T>>;
        if (elt == nullptr) {
            elt = new Type();
        }
        if constexpr (requires { elt->deserialize(mem, pos); }) {
            pos = elt->deserialize(mem, pos);
        } else {
            deserialize_(*elt);
        }
    }

    /* smart pointers *********************************************************/

    /// @brief Serialize function for smart pointers (unique and shared).
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::SmartPtr T>
    inline constexpr void serialize_(T &&elt) {
        if (elt != nullptr) {
            append('v');
            if constexpr (serializer::concepts::Serializable<
                              mtf::element_type_t<T>>) {
                elt->serialize(mem, pos);
            } else {
                serialize_(*elt);
            }
        } else {
            append('n');
        }
    }

    /// @brief Deserialize function for smart pointers (unique and shared). A
    ///        pointer is allocated if required.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::ConcreteSmartPtr T>
    inline constexpr void deserialize_(T &&elt) {
        using SP = mtf::base_t<T>;
        bool ptrValid = mem[pos++] == 'v';

        if (!ptrValid) {
            elt = nullptr;
            return;
        }
        static_assert(
            std::is_default_constructible_v<std::remove_pointer_t<SP>>,
            "The pointer types should be default constructible.");
        if constexpr (serializer::mtf::is_shared_v<SP>) {
            elt = std::make_shared<mtf::element_type_t<T>>();
        } else if constexpr (serializer::mtf::is_unique_v<SP>) {
            elt = std::make_unique<mtf::element_type_t<T>>();
        }
        if constexpr (requires { elt->deserialize(mem, pos); }) {
            elt->deserialize(mem, pos);
        } else {
            deserialize_(*elt);
        }
    }

    /* tuples *****************************************************************/

    /// @brief Helper function used to serialize tuples.
    /// @param tuple Tuple to serialize
    /// @param str String that will contain the result.
    template <class T, size_t... Idx>
    inline constexpr void serializeTuple(T &&tuple,
                                         std::index_sequence<Idx...>) {
        ([&] { serialize_(std::get<Idx>(tuple)); }(), ...);
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::TupleLike T>
        requires(!concepts::Trivial<T>)
    inline constexpr void serialize_(T &&tuple) {
        serializeTuple(
            tuple,
            std::make_index_sequence<std::tuple_size_v<mtf::base_t<T>>>());
    }

    /// @brief Helper function for deserializing tuples.
    /// @param str String that contains the data.
    template <class T, size_t... Idx>
    constexpr T deserializeTuple(std::index_sequence<Idx...>) {
        T tuple;
        ([&] { deserialize_(std::get<Idx>(tuple)); }(), ...);
        return tuple;
    }

    /// @brief Deserialize function tuples (std::tuple and std::pair).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::TupleLike T>
        requires(!concepts::Trivial<T>)
    inline constexpr void deserialize_(T &&elt) {
        elt = deserializeTuple<mtf::remove_const_tuple_t<mtf::base_t<T>>>(
            std::make_index_sequence<std::tuple_size_v<mtf::base_t<T>>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Enum T>
        requires(!concepts::Trivial<T>)
    inline constexpr void serialize_(T &&elt) {
        auto value = (std::underlying_type_t<mtf::base_t<T>>)elt;
        append(std::bit_cast<const byte_type *>(&value), sizeof(value));
    }

    /// @brief Deserialize function for enum types. The data is stored using the
    ///        underlying type.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Enum T>
        requires(!concepts::Trivial<T>)
    inline constexpr void deserialize_(T &&elt) {
        using Type = std::underlying_type_t<mtf::base_t<T>>;
        elt = (mtf::base_t<T>)*std::bit_cast<const Type *>(mem.data() + pos);
        pos += sizeof(Type);
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::String T>
    inline constexpr void serialize_(T &&elt) {
        using size_type = typename mtf::base_t<T>::size_type;
        size_type size = elt.size() + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));
        append(std::bit_cast<const byte_type *>(elt.data()), elt.size());
    }

    /// @brief Deserialize function for strings.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::String T>
    inline constexpr void deserialize_(T &&str) {
        using size_type = typename mtf::base_t<T>::size_type;
        size_type size = deserialize_size();
        str.assign(std::bit_cast<const char *>(mem.data() + pos), size);
        pos += size;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::Container T>
        requires(!concepts::Trivial<T>)
    inline constexpr void serialize_(T &&elts) {
        using ValueType =
            mtf::remove_const_t<serializer::mtf::iter_value_t<mtf::base_t<T>>>;
        using IterType = decltype(elts.begin());

        // append the size
        auto size = std::size(elts) + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));

        // if the type is trivial, the memory is serialized directly
        if constexpr (std::contiguous_iterator<IterType> &&
                      concepts::Trivial<ValueType>) {
            append(
                std::bit_cast<const byte_type *>(std::to_address(elts.begin())),
                sizeof(ValueType) * std::size(elts));
        } else {
            for (auto &elt : elts) {
                serialize_(elt);
            }
        }
    }

    /// @brief Deserialize function for containers. They must be iterable and
    ///        insertable (implements insert, add methods or usable with
    ///        std::insert).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::concepts::Container T>
        requires(!concepts::Trivial<T>)
    inline constexpr void deserialize_(T &&elts) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType =
            mtf::remove_const_t<serializer::mtf::iter_value_t<mtf::base_t<T>>>;
        using IterType = decltype(elts.begin());
        size_type size = deserialize_size();

        if constexpr (std::contiguous_iterator<IterType>) {
            if constexpr (concepts::Resizeable<T>) {
                elts.resize(size);
            }
            if constexpr (concepts::Trivial<ValueType>) {
                std::memcpy(
                    std::bit_cast<byte_type *>(std::to_address(elts.begin())),
                    mem.data() + pos, sizeof(ValueType) * size);
                pos += sizeof(ValueType) * size;
            } else {
                for (auto &elt : elts) {
                    deserialize_(elt);
                }
            }
        } else {
            if constexpr (concepts::Clearable<T>) {
                elts.clear();
            }
            for (size_t i = 0; i < size; ++i) {
                ValueType value{};
                deserialize_(value);
                if constexpr (serializer::concepts::Insertable<T, ValueType> ||
                              serializer::concepts::PushBackable<T,
                                                                 ValueType>) {
                    serializer::tools::insert(elts, std::move(value));
                } else {
                    serializer::tools::insert(elts, std::move(value), i);
                }
            }
        }
    }

    /* static array ***********************************************************/

    /// @brief Serialize function for static arrays.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::concepts::StaticArray T>
        requires(!concepts::Trivial<T>)
    inline constexpr void serialize_(T &&elt) {
        using SubType = std::remove_extent_t<mtf::base_t<T>>;
        size_t size = std::extent_v<mtf::base_t<T>>;

        if constexpr (!std::is_array_v<SubType> && concepts::Trivial<SubType>) {
            append(std::bit_cast<const byte_type *>(std::to_address(elt)),
                   sizeof(SubType) * size);
        } else {
            for (size_t i = 0; i < size; ++i) {
                serialize_(elt[i]);
            }
        }
    }

    /// @brief Deserialize function for static arrays.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <serializer::concepts::StaticArray T>
        requires(!concepts::Trivial<T>)
    inline constexpr void deserialize_(T &&elt) {
        using SubType = std::remove_extent_t<mtf::base_t<T>>;
        size_t size = std::extent_v<mtf::base_t<T>>;

        if constexpr (!std::is_array_v<SubType> && concepts::Trivial<SubType>) {
            std::memcpy(std::to_address(elt), mem.data() + pos,
                        sizeof(SubType) * size);
            pos += sizeof(SubType) * size;
        } else {
            for (size_t i = 0; i < size; ++i) {
                deserialize_(elt[i]);
            }
        }
    }

    /* dynamic array **********************************************************/

    /// @brief Serialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <concepts::Pointer T, typename DT, typename... DTs>
    inline constexpr void serialize_(tools::DynamicArray<T, DT, DTs...> elt) {
        using ST = std::remove_pointer_t<mtf::base_t<T>>;
        if (elt.mem == nullptr) {
            append('n');
            return;
        }
        append('v');

        if constexpr (std::is_pointer_v<ST>) {
            size_t size = (size_t)std::get<0>(elt.dimensions);
            for (size_t i = 0; i < size; ++i) {
                serialize_(tools::DynamicArray(
                    elt.mem[i], tools::tuplePopFront(elt.dimensions)));
            }
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            if constexpr (concepts::Trivial<ST>) {
                append(std::bit_cast<const byte_type *>(elt.mem),
                       size * sizeof(ST));
            } else {
                for (size_t i = 0; i < size; ++i) {
                    serialize_(elt.mem[i]);
                }
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
    template <concepts::Pointer T, typename DT, typename... DTs>
    inline constexpr void deserialize_(tools::DynamicArray<T, DT, DTs...> elt) {
        using ST = std::remove_pointer_t<mtf::base_t<T>>;
        bool ptrValid = mem[pos++] == 'v';

        if (!ptrValid) {
            elt.mem = nullptr;
            return;
        }

        if constexpr (std::is_pointer_v<ST>) {
            size_t size = (size_t)std::get<0>(elt.dimensions);
            if (elt.mem == nullptr) {
                elt.mem = new ST[size]();
            }
            for (size_t i = 0; i < size; ++i) {
                deserialize_(tools::DynamicArray(
                    elt.mem[i], tools::tuplePopFront(elt.dimensions)));
            }
        } else {
            size_t size = tools::tupleProd<size_t>(elt.dimensions);
            if (elt.mem == nullptr) {
                elt.mem = new ST[size]();
            }
            if constexpr (concepts::Trivial<ST>) {
                std::memcpy(elt.mem, mem.data() + pos, size * sizeof(ST));
                pos += size * sizeof(ST);
            } else {
                for (size_t i = 0; i < size; ++i) {
                    deserialize_(elt.mem[i]);
                }
            }
        }
    }
};

} // namespace serializer

#endif
