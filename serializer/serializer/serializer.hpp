#ifndef SERIALIZER_SERIALIZER_SERIALIZER_HPP
#define SERIALIZER_SERIALIZER_SERIALIZER_HPP
#include "../exceptions/unsupported_type.hpp"
#include "../meta/serializer_meta.hpp"
#include "../meta/type_check.hpp"
#include "../meta/type_transform.hpp"
#include "../tools/dynamic_array.hpp"
#include "../tools/tools.hpp"
#include "../tools/type_table.hpp"
#include "serialize.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

/// @brief namespace serializer
namespace serializer {

/// @brief Default serializer that contains the functions for serializing and
///        deserializing types from the standard library. It inherits from the
///        convert behavior for additional types so the user can add its own
///        functions.
/// @tparam MemT Type of the memory buffer.
/// @tparam AdditionalTypes External types for which the user can add support.
template <typename MemT, typename TypeTable = tools::TypeTable<>,
          typename... AdditionalTypes>
struct Serializer : Serialize<AdditionalTypes>... {
    MemT &mem;      ///< memory buffer in which the serialized data are stored
    size_t pos = 0; ///< position in the memory buffer.

    using type_table = TypeTable;
    using id_type = typename TypeTable::id_type;
    using mem_type = MemT; ///< alias to the type of the momory buffer
    using byte_type =
        std::remove_cvref_t<decltype(mem[0])>; ///< alias to the byte type

    /* Constructor ************************************************************/

    /// @brief Constructor from memory buffer reference and position.
    /// @param mem Memory buffer in which the data is serialized.
    /// @param pos Position in the memory buffer.
    constexpr Serializer(MemT &mem, size_t pos = 0) : mem(mem), pos(pos) {}

    /* helper functions *******************************************************/

    /// @brief Helper function for deserializing the size of containers.
    /// @tparam Type of the size
    /// @return Deserialized size.
    template <typename T> inline constexpr T deserializeSize() {
        auto size = *std::bit_cast<const T *>(mem.data() + pos);
        pos += sizeof(T);
        return size - 1;
    }

    /// @brief Append a buffer of byptes to the memory (mem and pos are changed)
    /// @param bytes Buffer of bytes.
    /// @param nbBytes Size of the buffer.
    inline constexpr void append(const byte_type *bytes, size_t nbBytes) {
        if constexpr (!std::is_const_v<MemT>) {
            if constexpr (mtf::is_serializer_bytes_v<mtf::clean_t<mem_type>>) {
                mem.append(pos, bytes, nbBytes);
                pos += nbBytes;
            } else {
                if (mem.size() < pos + nbBytes) [[unlikely]] {
                    if constexpr (concepts::Resizeable<mem_type>) {
                        mem.resize((mem.size() + nbBytes) * 2);
                    } else {
                        throw std::out_of_range(
                            "error: the serialization array is too small.");
                    }
                }
                std::memcpy(mem.data() + pos, bytes, nbBytes);
                pos += nbBytes;
            }
        }
    }

    /// @brief Helper function for appending a simple elements to the memory
    ///        buffer.
    /// @param elt element to append.
    inline constexpr void append(auto &&elt) {
        append(std::bit_cast<const byte_type *>(&elt), sizeof(elt));
    }

    /// @brief Deserialize an identifier.
    /// @param elt Element that is deserialized.
    /// @return id
    inline constexpr id_type deserializeId(auto &&elt) {
        auto id = *std::bit_cast<const id_type *>(mem.data() + pos);
        // if the id is managed automatically, we have to update the pos,
        // otherwise, it will be deserialized with the attributes.
        if constexpr (
            requires { elt.typeId(); } || requires { elt->typeId(); }) {
            pos += sizeof(id_type);
        }
        return id;
    }

    /* types with ids *********************************************************/

    /// @brief Serialize a generic type registed in the type table
    /// @param elt Element serialized.
    template <typename T>
        requires(tools::has_type_v<T, TypeTable>)
    inline constexpr void serialize_(T &&elt) {
        if constexpr (requires { elt->serialize(mem, pos); }) {
            pos = elt->serialize(mem, pos);
        } else if constexpr (requires { elt.serialize(mem, pos); }) {
            pos = elt.serialize(mem, pos);
        }
    }

    /// @brief Serialize a generic type registed in the type table
    /// @param elt Element serialized.
    template <typename T>
        requires(tools::has_type_v<T, TypeTable>)
    inline constexpr void deserialize_(T &&elt) {
        auto id = deserializeId(elt);
        tools::createId<TypeTable>(id, elt);
        if constexpr (requires { elt.deserialize(mem, pos); }) {
            pos = elt.deserialize(mem, pos);
        } else if constexpr (requires { elt->deserialize(mem, pos); }) {
            pos = elt->deserialize(mem, pos);
        }
    }

    /* no automatic serialization types (custom convertor) ********************/

    /// @brief Fallback functions for non serializable types. Here either we use
    ///        a custom function if it has be implemented or we throw an
    ///        exception.
    ///        Note: exceptions are a way to control the display of the error
    ///        message. We could use a static assert but the message will be
    ///        lost in the hole compiler error output.
    /// @param elt Element that is serialized.
    template <typename T>
        requires(concepts::NonAutomaticSerialize<T, MemT, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void serialize_(T &&elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            static_cast<Serialize<mtf::clean_t<T>> *>(this)->serialize(elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<
                mtf::clean_t<T>>();
        }
    }

    /// @brief Fallback functions for non deserializable types. Here either we
    ///        use a custom function if it has be implemented or we throw an
    ///        exception.
    ///        Note: exceptions are a way to control the display of the error
    ///        message. We could use a static assert but the message will be
    ///        lost in the hole compiler error output.
    /// @param elt Element that is deserialized.
    template <typename T>
        requires(
            concepts::NonAutomaticDeserialize<T, MemT, AdditionalTypes...> &&
            !tools::has_type_v<T, TypeTable>)
    inline constexpr void deserialize_(T &&elt) {
        if constexpr (mtf::contains_v<T, AdditionalTypes...>) {
            // we need a static cast because of implicit constructors (ex:
            // pointer to shared_ptr)
            static_cast<Serialize<mtf::clean_t<T>> *>(this)->deserialize(elt);
        } else {
            throw serializer::exceptions::UnsupportedTypeError<
                mtf::clean_t<T>>();
        }
    }

    /* serializable ***********************************************************/

    /// @brief Serialize function for the serializable types (they have a
    ///        serialize method).
    /// @param elt Element that is serialized.
    template <typename T>
        requires(concepts::UseSerialize<T, MemT, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void serialize_(T &&elt) {
        pos = elt.serialize(mem, pos);
    }

    /// @brief Deserialize function for the deserializable types (they have a
    ///        deserialize method).
    /// @param elt Element that is deserialized.
    template <typename T>
        requires(concepts::UseDeserialize<T, MemT, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void deserialize_(T &&elt) {
        pos = elt.deserialize(mem, pos);
    }

    /* trivial types **********************************************************/

    /// @brief Serialize function for the trivial types.
    /// @param elt Element that is serialized.
    template <serializer::concepts::Trivial T>
        requires(!concepts::Serializable<T, MemT>)
    inline constexpr void serialize_(T &&elt) {
        append(std::bit_cast<const byte_type *>(&elt), sizeof(elt));
    }

    /// @brief Deserialize function for the trivial types.
    /// @param elt Element that is deserialized.
    template <serializer::concepts::Trivial T>
        requires(!concepts::Deserializable<T, MemT>)
    inline constexpr void deserialize_(T &&elt) {
        elt = *std::bit_cast<const mtf::clean_t<T> *>(mem.data() + pos);
        pos += sizeof(T);
    }

    /* pointers ***************************************************************/

    /// @brief Serialize function for the pointer types.
    ///        The pointer should be valid (nullptr or value).
    /// @param elt Element that is serialized.
    template <serializer::concepts::Pointer T>
        requires(!mtf::contains_v<T, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void serialize_(T &&elt) {
        if (elt == nullptr) {
            append('n');
            return;
        }
        append('v');
        if constexpr (requires { elt->serialize(mem, pos); }) {
            pos = elt->serialize(mem, pos);
        } else {
            serialize_(*elt);
        }
    }

    /// @brief Deserialize function for the pointer types. If the pointer is not
    ///        null, a dynamic allocation is perfsset 56973 has been removed
    ///        Waiver Form PIV LONG TERM Eormed before deserializing the result.
    ///        This memory should be handled by the user.
    /// @param elt Element that is deserialized.
    template <serializer::concepts::ConcretePtr T>
        requires(!mtf::contains_v<T, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void deserialize_(T &&elt) {
        bool ptrValid = char(mem[pos++]) == 'v';

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
    /// @param elt Element that is serialized.
    template <serializer::concepts::SmartPtr T>
        requires(!mtf::contains_v<T, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void serialize_(T &&elt) {
        using ST = mtf::element_type_t<T>;
        if (elt != nullptr) {
            append('v');
            if constexpr (concepts::Serializable<ST, MemT>) {
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
    /// @param elt Element that is deserialized.
    template <serializer::concepts::ConcreteSmartPtr T>
        requires(!mtf::contains_v<T, AdditionalTypes...> &&
                 !tools::has_type_v<T, TypeTable>)
    inline constexpr void deserialize_(T &&elt) {
        bool ptrValid = char(mem[pos++]) == 'v';

        if (!ptrValid) {
            elt = nullptr;
            return;
        }
        static_assert(mtf::is_default_constructible_v<T>,
                      "The pointer types should be default constructible.");
        if constexpr (serializer::mtf::is_shared_v<T>) {
            elt = std::make_shared<mtf::element_type_t<T>>();
        } else if constexpr (serializer::mtf::is_unique_v<T>) {
            elt = std::make_unique<mtf::element_type_t<T>>();
        }
        if constexpr (concepts::Deserializable<T, MemT>) {
            elt->deserialize(mem, pos);
        } else {
            deserialize_(*elt);
        }
    }

    /* tuples *****************************************************************/

    /// @brief Helper function used to serialize tuples.
    /// @param tuple Tuple to serialize.
    /// @param index_sequence Indicies of the tuple elements.
    template <size_t... Idx>
    inline constexpr void serializeTuple(auto &&elt,
                                         std::index_sequence<Idx...>) {
        ([&] { serialize_(std::get<Idx>(elt)); }(), ...);
    }

    /// @brief Serialize function for tuples (std::tuple and std::pair).
    /// @param elt Element that is serialized.
    template <serializer::concepts::TupleLike T>
    inline constexpr void serialize_(T &&elt) {
        serializeTuple(
            elt,
            std::make_index_sequence<std::tuple_size_v<mtf::clean_t<T>>>());
    }

    /// @brief Helper function for deserializing tuples.
    /// @param index_sequence Indicies of the tuple elements.
    template <size_t... Idx>
    constexpr void deserializeTuple(auto &&elt, std::index_sequence<Idx...>) {
        ([&] { deserialize_(std::get<Idx>(elt)); }(), ...);
    }

    /// @brief Deserialize function tuples (std::tuple and std::pair).
    /// @param elt Element that is deserialized.
    template <serializer::concepts::TupleLike T>
    inline constexpr void deserialize_(T &&elt) {
        deserializeTuple(
            elt,
            std::make_index_sequence<std::tuple_size_v<mtf::clean_t<T>>>());
    }

    /* enums ******************************************************************/

    /// @brief Serialize function for the enum types. The underlying type is
    ///        used to store the data.
    /// @param elt Element that is serialized.
    template <serializer::concepts::Enum T>
        requires(!concepts::Trivial<T>)
    inline constexpr void serialize_(T &&elt) {
        auto value = (std::underlying_type_t<mtf::clean_t<T>>)elt;
        append(std::bit_cast<const byte_type *>(&value), sizeof(value));
    }

    /// @brief Deserialize function for enum types. The data is stored using the
    ///        underlying type.
    /// @param elt Element that is deserialized.
    template <serializer::concepts::Enum T>
        requires(!concepts::Trivial<T>)
    inline constexpr void deserialize_(T &&elt) {
        using Type = std::underlying_type_t<mtf::clean_t<T>>;
        elt = (mtf::clean_t<T>)*std::bit_cast<const Type *>(mem.data() + pos);
        pos += sizeof(Type);
    }

    /* strings ****************************************************************/

    /// @brief Serialize function for strings.
    /// @param elt Element that is serialized.
    template <serializer::concepts::String T>
    inline constexpr void serialize_(T &&elt) {
        using size_type = typename mtf::clean_t<T>::size_type;
        size_type size = elt.size() + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));
        append(std::bit_cast<const byte_type *>(elt.data()), elt.size());
    }

    /// @brief Deserialize function for strings.
    /// @param elt Element that is deserialized.
    template <serializer::concepts::String T>
    inline constexpr void deserialize_(T &&str) {
        using size_type = typename mtf::clean_t<T>::size_type;
        size_type size = deserializeSize<size_type>();
        str.assign(std::bit_cast<const char *>(mem.data() + pos), size);
        pos += size;
    }

    /* iterable containers ****************************************************/

    /// @brief Serialize function for containers. They must be iterable.
    /// @param elt Element that is serialized.
    template <serializer::concepts::Container T>
        requires(!concepts::Trivial<T> && !concepts::Serializable<T, MemT>)
    inline constexpr void serialize_(T &&elts) {
        using ValueType =
            mtf::remove_const_t<mtf::iter_value_t<mtf::clean_t<T>>>;

        // append the size
        auto size = std::size(elts) + 1;
        append(std::bit_cast<const byte_type *>(&size), sizeof(size));

        // if the type is trivial, the memory is serialized directly
        if constexpr (concepts::ContiguousTrivial<T, MemT>) {
            append(
                std::bit_cast<const byte_type *>(std::to_address(elts.begin())),
                sizeof(ValueType) * std::size(elts));
        } else {
            for (auto &elt : elts) {
                serialize_(elt);
            }
        }
    }

    /// @brief Deserialize function for containers..
    /// @param elt Element that is deserialized.
    template <serializer::concepts::Container T>
        requires(!concepts::Trivial<T> && !concepts::Deserializable<T, MemT>)
    inline constexpr void deserialize_(T &&elts) {
        using size_type = decltype(std::size(std::declval<T>()));
        using ValueType =
            mtf::remove_const_t<mtf::iter_value_t<mtf::clean_t<T>>>;
        using IterType = decltype(elts.begin());
        size_type size = deserializeSize<size_type>();

        if constexpr (concepts::ContiguousResizeable<T>) {
            elts.resize(size);
        } else if constexpr (concepts::Clearable<T>) {
            elts.clear();
        }

        if constexpr (concepts::ContiguousTrivial<T, MemT>) {
            std::memcpy(
                std::bit_cast<byte_type *>(std::to_address(elts.begin())),
                mem.data() + pos, sizeof(ValueType) * size);
            pos += sizeof(ValueType) * size;
        } else if constexpr (std::contiguous_iterator<IterType>) {
            for (auto &elt : elts) {
                deserialize_(elt);
            }
        } else {
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
    /// @param elt Element that is serialized.
    template <serializer::concepts::StaticArray T>
    inline constexpr void serialize_(T &&elt) {
        size_t size = std::extent_v<mtf::clean_t<T>>;

        if constexpr (concepts::TrivialySerializableStaticArray<T, MemT>) {
            append(std::bit_cast<const byte_type *>(std::to_address(elt)),
                   sizeof(elt[0]) * size);
        } else {
            for (size_t i = 0; i < size; ++i) {
                serialize_(elt[i]);
            }
        }
    }

    /// @brief Deserialize function for static arrays.
    /// @param elt Element that is deserialized.
    template <serializer::concepts::StaticArray T>
    inline constexpr void deserialize_(T &&elt) {
        using ST = std::remove_extent_t<mtf::clean_t<T>>;
        size_t size = std::extent_v<mtf::clean_t<T>>;

        if constexpr (concepts::TrivialyDeserializableStaticArray<T, MemT>) {
            std::memcpy(std::to_address(elt), mem.data() + pos,
                        sizeof(ST) * size);
            pos += sizeof(ST) * size;
        } else {
            for (size_t i = 0; i < size; ++i) {
                deserialize_(elt[i]);
            }
        }
    }

    /* dynamic array **********************************************************/

    /// @brief Serialize function for dynamic arrays (dynamic arrays should be
    ///        wrap in the DynamicArray type).
    /// @param elt Element that is serialized.
    template <concepts::Pointer T, typename DT, typename... DTs>
    inline constexpr void serialize_(tools::DynamicArray<T, DT, DTs...> elt) {
        using ST = std::remove_pointer_t<mtf::clean_t<T>>;
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
            if constexpr (concepts::Trivial<ST> &&
                          !concepts::Serializable<ST, MemT>) {
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
    /// @param elt Element that is deserialized.
    template <concepts::Pointer T, typename DT, typename... DTs>
    inline constexpr void deserialize_(tools::DynamicArray<T, DT, DTs...> elt) {
        using ST = std::remove_pointer_t<mtf::clean_t<T>>;
        bool ptrValid = char(mem[pos++]) == 'v';

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
            if constexpr (concepts::Trivial<ST> &&
                          !concepts::Deserializable<ST, MemT>) {
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
