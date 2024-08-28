#ifndef SERIALIZER_SERIALIZE_H
#define SERIALIZER_SERIALIZE_H
#include "convertor/convertor.hpp"
#include "meta/concepts.hpp"
#include "tools/context.hpp"
#include "tools/type_table.hpp"
#include "tools/bytes.hpp"
#include "tools/super.hpp"

// TODO: use always inline attribute

/// @brief serializer namespace
namespace serializer {

/// @brief Serialize the arguments into the memory buffer at the given position.
/// @tparam Conv Convertor type.
/// @param mem  Buffer in which the serialized data will be stored.
/// @param pos  Start position in the buffer for serializing the data.
/// @param args Values to serialize
template <typename Conv>
inline constexpr size_t serialize(auto &mem, size_t pos, auto &&...args) {
    using mem_t = decltype(mem);
    [[maybe_unused]] bool first_level = pos == 0;

    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (SerializerFunction(args, conv)) {
                args(tools::Context<tools::Phases::Serialization,
                                    decltype(conv)>(conv));
            } else {
                conv.serialize_(args);
            }
        }(),
        ...);
    if constexpr (!mtf::is_vec_v<mem_t> && concepts::Resizeable<mem_t>) {
        if (first_level) [[unlikely]] {
            mem.resize(conv.pos);
        }
    }
    return conv.pos;
}

/// @brief Deserialize the arguments from the memory buffer at the given
///        position.
/// @tparam Conv Convertor type.
/// @param mem  Buffer in which the serialized data is be stored.
/// @param pos  Start position in the buffer for deserializing the data.
/// @param args references to the variables that are deserialized.
template <typename Conv>
inline constexpr size_t deserialize(auto &mem, size_t pos, auto &&...args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (SerializerFunction(args, conv)) {
                args(tools::Context<tools::Phases::Deserialization,
                                    decltype(conv)>(conv));
            } else {
                conv.deserialize_(args);
            }
        }(),
        ...);
    return conv.pos;
}

/// @brief Serialize obj into mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for serializing the data.
/// @param obj Pointer to the object that is serialized.
template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::is_trivially_copy_assignable_v<T>)
inline constexpr size_t serializeStruct(auto &mem, size_t pos, T const *obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    Convertor<decltype(mem)> conv(mem, pos);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    conv.append(std::bit_cast<const byte_type *>(obj), nb_bytes);
    return conv.pos;
}

/// @brief Deserialize obj from mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for deserializing the data.
/// @param obj Pointer to the object that is deserialized.
template <typename T>
inline constexpr size_t deserializeStruct(auto &mem, size_t pos, T *obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

/// @brief Create a function for serializing the given object using the
///        accessors.
/// @param obj       Object to serialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
constexpr inline auto bindSerialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        serializer::serialize<serializer::Convertor<decltype(mem)>>(
            mem, pos, std::invoke(accessors, obj)...);
    };
}

/// @brief Create a function for deserializing the given object using the
///        accessors.
/// @param obj       Object to deserialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
constexpr inline auto bindDeserialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        serializer::deserialize<serializer::Convertor<decltype(mem)>>(
            mem, pos, std::invoke(accessors, obj)...);
    };
}

// TODO: change this
using default_mem_type = tools::Bytes<uint8_t>;
/* using default_mem_type = std::vector<uint8_t>; */

} // end namespace serializer

/// @brief Generate the serialze and deserialize methods with the specified
///        convertor.
/// @param Conv Convertor.
/// @param ... Members to serialize.
#define SERIALIZE_CONV(Conv, ...)                                              \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize<Conv<decltype(mem)>>(mem, pos,            \
                                                          __VA_ARGS__);        \
    }                                                                          \
    constexpr size_t deserialize(auto &mem, size_t pos = 0) {                  \
        return serializer::deserialize<Conv<decltype(mem)>>(mem, pos,          \
                                                            __VA_ARGS__);      \
    }

/// @brief Generate the serialize and deserialize methods with the default
///        convertor.
/// @param ... Members to serialize.
#define SERIALIZE(...) SERIALIZE_CONV(serializer::Convertor, __VA_ARGS__)

/// @brief Generate the serialize and deserialize methods with the specified
///        convertor. The keywords virtual and override can be added.
/// @param Conv Convertor.
/// @param virt Virtual keyworkd.
/// @param over Override keyworkd.
/// @param ... Members to serialize.
#define __SERIALIZE__(Conv, virt, over, ...)                                   \
    constexpr virt size_t serialize(typename Conv::mem_type &mem,              \
                                    size_t pos = 0) const over {               \
        return serializer::serialize<Conv>(mem, pos, __VA_ARGS__);             \
    }                                                                          \
    constexpr virt size_t deserialize(typename Conv::mem_type &mem,            \
                                      size_t pos = 0) over {                   \
        return serializer::deserialize<Conv>(mem, pos, __VA_ARGS__);           \
    }

/// @brief Generate the serialze and deserialize virtual methods using the
///        specified convertor.
/// @param Conv Convertor
/// @param ... Members to serialize
#define VIRTUAL_SERIALIZE(Conv, ...)                                           \
    __SERIALIZE__(Conv, virtual, /* over */, __VA_ARGS__)

/// @brief Generate the serialze and deserialize override methods using the
///        specified convertor.
/// @param Conv Convertor
/// @param ... Members to serialize
#define SERIALIZE_OVERRIDE(Conv, ...)                                          \
    __SERIALIZE__(Conv, /* virt */, override, __VA_ARGS__)

/// @brief Generate empty serialize and deserialize virtual methods (useful
///        whithin a concrete super class).
/// @param MemT Type of the buffer (virtual methods cannot be template).
#define SERIALIZE_EMPTY(MemT)                                                  \
    constexpr virtual size_t serialize(MemT &, size_t = 0) const { return 0; } \
    constexpr virtual size_t deserialize(MemT &, size_t = 0) { return 0; }

/// @brief Generate abstract serialize and deserialize virtual methods.
/// @param MemT Type of the buffer (virtual methods cannot be template).
#define SERIALIZE_ABSTRACT(MemT)                                               \
    constexpr virtual size_t serialize(MemT &, size_t = 0) const = 0;          \
    constexpr virtual size_t deserialize(MemT &, size_t = 0) = 0;

/// @brief Generate the serialize and deserialize methods that use the
///        (de)serialize functions.
#define SERIALIZE_STRUCT()                                                     \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serializeStruct(mem, pos, this);                    \
    }                                                                          \
    constexpr size_t deserialize(auto const &mem, size_t pos = 0) {            \
        return serializer::deserializeStruct(mem, pos, this);                  \
    }

#endif
