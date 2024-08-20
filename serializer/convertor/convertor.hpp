#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "../exceptions/unsupported_type.hpp"
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

    size_t pos = 0;

    constexpr Convertor(MemT &mem, size_t pos = 0) : mem(mem), pos(pos) {}

    constexpr size_t deserialize_size() {
        size_t size = *std::bit_cast<const size_t *>(mem.data() + pos);
        pos += sizeof(size);
        return size - 1;
    }

    constexpr void append(char chr) {
        mem.append(pos++, std::bit_cast<const byte_type *>(&chr), 1);
    }

    constexpr void append(const byte_type *bytes, size_t nb_bytes) {
        // size not changed correctly:
        /* if ((spos + nb_bytes) >= mem.capacity()) { */
        /*     mem.resize(std::max(mem.capacity() * 2, spos + nb_bytes)); */
        /* } */
        /* std::memcpy(mem.data() + spos, bytes, nb_bytes); */
        /* spos += nb_bytes; */

        // 1164 / 882
        /* for (size_t i = 0; i < nb_bytes; ++i) { */
        /*   mem.push_back(bytes[i]); */
        /* } */

        // 2112 / 901
        /* std::copy(bytes, bytes + nb_bytes, std::back_inserter(mem)); */

        // 5992 / 876
        /* mem.insert(mem.begin() + spos, bytes, bytes + nb_bytes); */
        /* spos += nb_bytes; */

        /* custom vector */

        // 673 / 815
        /* mem.insert(spos, bytes, nb_bytes); */
        /* spos += nb_bytes; */

        // 673 / 815
        mem.append(pos, bytes, nb_bytes);
        pos += nb_bytes;

        // 641 / 842
        /* mem.append(bytes, nb_bytes); */
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
    constexpr void serialize_(T &&elt) {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            static_cast<const Convert<T> *>(this)->serialize(elt, mem);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<
                std::remove_reference_t<T>>();
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
    constexpr void deserialize_(T &&elt) {
        if constexpr (tools::mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            // TODO: rework convert functions
            static_cast<Convert<T> *>(this)->deserialize(elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<
                std::remove_reference_t<T>>();
        }
    }

    /* serializable ***********************************************************/

    /// @brief Serialize function for the serializable types (they have a
    ///        serialize method).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Serializable T>
    constexpr void serialize_(T &&elt) {
        pos = elt.serialize(mem, pos);
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Deserializable T>
    constexpr void deserialize_(T &&elt) {
        pos = elt.deserialize(mem, pos);
    }

    /* fundamental types ******************************************************/

    /// @brief Serialize function for the fundamental types.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Fundamental T>
    constexpr void serialize_(T &&elt) {
        append(std::bit_cast<const byte_type *>(&elt), sizeof(T));
    }

    /// @brief Deserialize function for the deserializable fundamental types.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Fundamental T>
    constexpr void deserialize_(T &&elt) {
        elt = *std::bit_cast<const tools::mtf::base_t<T> *>(mem.data() + pos);
        pos += sizeof(T);
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Pointer T>
    constexpr void serialize_(T &&elt) {
        if (elt != nullptr) {
            append('v');
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {
                elt->serialize(mem, pos);
            } else {
                serialize_(*elt);
            }
        } else {
            append('n');
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
    constexpr void deserialize_(T &&elt) {
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
        if constexpr (std::is_fundamental_v<Type>) {
            deserialize_(*elt);
        } else {
            elt->deserialize(mem, pos);
        }
    }

    /* smart pointers *********************************************************/

    /// @brief Serialize function for smart pointers (unique and shared).
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::SmartPtr T>
    constexpr void serialize_(T &&elt) {
        using SP = tools::mtf::base_t<T>;
        if (elt != nullptr) {
            append('v');
            if constexpr (serializer::tools::concepts::Serializable<
                              typename SP::element_type>) {
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
    template <serializer::tools::concepts::ConcreteSmartPtr T>
    constexpr void deserialize_(T &&elt) {
        using SP = tools::mtf::base_t<T>;
        bool ptrValid = mem[pos++] == 'v';

        if (!ptrValid) {
            elt = nullptr;
            return;
        }
        static_assert(
            std::is_default_constructible_v<std::remove_pointer_t<SP>>,
            "The pointer types should be default constructible.");
        if constexpr (serializer::tools::mtf::is_shared_v<SP>) {
            elt = std::make_shared<typename SP::element_type>();
        } else if constexpr (serializer::tools::mtf::is_unique_v<SP>) {
            elt = std::make_unique<typename SP::element_type>();
        }
        if constexpr (serializer::tools::concepts::Deserializable<
                          typename SP::element_type>) {
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
    constexpr void serializeTuple(T &&tuple, std::index_sequence<Idx...>) {
        ([&] { serialize_(std::get<Idx>(tuple)); }(), ...);
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::TupleLike T>
        requires(!serializer::tools::concepts::Array<T>)
    constexpr void serialize_(T &&tuple) {
        serializeTuple(tuple, std::make_index_sequence<
                                  std::tuple_size_v<tools::mtf::base_t<T>>>());
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
    template <serializer::tools::concepts::TupleLike T>
        requires(!serializer::tools::concepts::Array<T>)
    constexpr void deserialize_(T &&elt) {
        elt = deserializeTuple<
            tools::mtf::remove_const_tuple_t<tools::mtf::base_t<T>>>(
            std::make_index_sequence<
                std::tuple_size_v<tools::mtf::base_t<T>>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::Enum T>
    constexpr void serialize_(T &&elt) {
        auto value = (std::underlying_type_t<tools::mtf::base_t<T>>)elt;
        append(std::bit_cast<const byte_type *>(&value), sizeof(value));
    }

    /// @brief Deserialize function for enum types. The data is stored using the
    ///        underlying type.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize. It it
    ///            just used for creating an overload of the deserialize
    ///            function.
    template <serializer::tools::concepts::Enum T>
    constexpr void deserialize_(T &&elt) {
        using Type = std::underlying_type_t<tools::mtf::base_t<T>>;
        elt = (tools::mtf::base_t<T>) *std::bit_cast<const Type *>(mem.data() + pos);
        pos += sizeof(Type);
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::String T>
    constexpr void serialize_(T &&elt) {
        using size_type = typename tools::mtf::base_t<T>::size_type;
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
    constexpr void deserialize_(T &&str) {
        using size_type = typename tools::mtf::base_t<T>::size_type;
        size_type size = deserialize_size();
        str.assign(std::bit_cast<const char *>(mem.data() + pos), size);
        pos += size;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::NonStringIterable T>
    constexpr void serialize_(T &&elts) {
        auto size = std::size(elts) + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));
        for (auto &&elt : elts) {
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
    constexpr void deserialize_(T &&elt) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType = tools::concepts::remove_const_t<
            serializer::tools::mtf::iter_value_t<tools::mtf::base_t<T>>>;
        size_t idx = 0;
        size_type size = deserialize_size();
        auto insert = [&elt, &idx](ValueType &&value) {
            if constexpr (serializer::tools::concepts::Insertable<T,
                                                                  ValueType> ||
                          serializer::tools::concepts::PushBackable<
                              T, ValueType>) {
                (void)idx;
                serializer::tools::insert(elt, std::move(value));
            } else {
                serializer::tools::insert(elt, std::move(value), idx++);
            }
        };

        if constexpr (tools::concepts::Clearable<T>) {
            elt.clear();
        }

        for (size_t i = 0; i < size; ++i) {
            ValueType value;
            deserialize_(value);
            insert(std::move(value));
        }
    }

    /* static array ***********************************************************/

    /// @brief Serialize function for static arrays.
    /// @param elt Reference to the element that we want to serialize.
    /// @param str String that contains the result.
    template <serializer::tools::concepts::StaticArray T>
    constexpr void serialize_(T &&elt) {
        for (size_t i = 0; i < std::extent_v<T>; ++i) {
            serialize_(elt[i]);
        }
    }

    /// @brief Deserialize function for static arrays.
    /// @param str String view of the data.
    /// @param elt Reference to the element that we want to deserialize.
    template <serializer::tools::concepts::StaticArray T>
    constexpr void deserialize_(T &&elt) {
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
    constexpr void serialize_(tools::DynamicArray<T, DT, DTs...> &&elt) {
        using ST = std::remove_pointer_t<T>;
        if (elt.mem == nullptr) {
            append("n", 1);
            return;
        }
        append("v", 1);

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
    constexpr void deserialize_(tools::DynamicArray<T, DT, DTs...> &&elt) {
        using ST = std::remove_pointer_t<T>;
        bool ptrValid = mem[pos++] == 'v';

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
