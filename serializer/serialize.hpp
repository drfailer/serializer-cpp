#ifndef SERIALIZER_SERIALIZE_H
#define SERIALIZER_SERIALIZE_H
#include "meta/concepts.hpp"
#include "serializer/serializer.hpp"
#include "tools/context.hpp"

/// @brief serializer namespace
namespace serializer {

/******************************************************************************/
/*                          serialize / deserialize                           */
/******************************************************************************/

/// @brief Serialize the arguments into the memory buffer at the given position.
/// @tparam Ser Serializer type.
/// @param mem  Buffer in which the serialized data will be stored.
/// @param pos  Start position in the buffer for serializing the data.
/// @param args Values to serialize
/// @return Position of the next element in the buffer.
template <typename Ser>
inline constexpr size_t serialize(auto &mem, size_t pos, auto &&...args) {
    using mem_t = decltype(mem);
    [[maybe_unused]] bool first_level = pos == 0;

    Ser serializer(mem, pos);
    (
        [&serializer, &args] {
            if constexpr (SerializerFunction(args, serializer)) {
                args(tools::Context<tools::Phases::Serialization,
                                    decltype(serializer)>(serializer));
            } else {
                serializer.serialize_types(args);
            }
        }(),
        ...);
    if constexpr (!mtf::is_serializer_bytes_v<mem_t> &&
                  concepts::Resizeable<mem_t>) {
        if (first_level) [[unlikely]] {
            mem.resize(serializer.pos);
        }
    }
    return serializer.pos;
}

/// @brief Deserialize the arguments from the memory buffer at the given
///        position.
/// @tparam Ser Serializer type.
/// @param mem  Buffer in which the serialized data is be stored.
/// @param pos  Start position in the buffer for deserializing the data.
/// @param args references to the variables that are deserialized.
/// @return Position of the next element in the buffer.
template <typename Ser>
inline constexpr size_t deserialize(auto &mem, size_t pos, auto &&...args) {
    Ser serializer(mem, pos);
    (
        [&serializer, &args] {
            if constexpr (SerializerFunction(args, serializer)) {
                args(tools::Context<tools::Phases::Deserialization,
                                    decltype(serializer)>(serializer));
            } else {
                serializer.deserialize_types(args);
            }
        }(),
        ...);
    return serializer.pos;
}

/******************************************************************************/
/*                      serialize / deserialize with id                       */
/******************************************************************************/

/// @brief Serialize the members and the id if required (if the type is not
///        present in the type table, the id is not serialized).
/// @tparam Ser Serializer type.
/// @tparam T Type serialize (used for the id).
/// @param mem Buffer of bytes that will contain the serialized data.
/// @param pos Position in mem.
/// @param args Elements that are serialized.
template <typename Ser, typename T>
constexpr inline size_t serializeWithId(auto &mem, size_t pos, auto &&...args) {
    if constexpr (tools::has_type_v<T, typename Ser::type_table>) {
        return serialize<Ser>(
            mem, pos, tools::getId<T>(typename Ser::type_table()), args...);
    } else {
        return serialize<Ser>(mem, pos, args...);
    }
}

/// @brief Deserialize the members and the id if required (if the type is not
///        present in the type table, the id is not serialized).
/// @tparam Ser Serializer type.
/// @tparam T Type serialize (used for the id).
/// @param mem Buffer of bytes that contains the serialized data.
/// @param pos Position in mem.
/// @param args Elements that are deserialized.
template <typename Ser, typename T>
constexpr inline size_t deserializeWithId(auto &mem, size_t pos,
                                          auto &&...args) {
    if constexpr (tools::has_type_v<T, typename Ser::type_table>) {
        return deserialize<Ser>(
            mem, pos, tools::getId<T>(typename Ser::type_table()), args...);
    } else {
        return deserialize<Ser>(mem, pos, args...);
    }
}

/******************************************************************************/
/*                       serialize / deserialize struct                       */
/******************************************************************************/

/// @brief Serialize obj into mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for serializing the data.
/// @param obj Pointer to the object that is serialized.
/// @return Position of the next element in the buffer.
template <typename T>
    requires(std::is_trivially_copyable_v<T> &&
             std::is_trivially_copy_assignable_v<T>)
inline constexpr size_t serializeStruct(auto &mem, size_t pos, T const *obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    Serializer<decltype(mem)> serializer(mem, pos);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    serializer.append(std::bit_cast<const byte_type *>(obj), nb_bytes);
    return serializer.pos;
}

/// @brief Deserialize obj from mem at pos using a bit_cast.
/// @tparam T Object type (this).
/// @param mem Buffer in which the serialized data is be stored.
/// @param pos Start position in the buffer for deserializing the data.
/// @param obj Pointer to the object that is deserialized.
/// @return Position of the next element in the buffer.
template <typename T>
inline constexpr size_t deserializeStruct(auto &mem, size_t pos, T *obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

/******************************************************************************/
/*                        bind serialize / deserialize                        */
/******************************************************************************/

/// @brief Create a function for serializing the given object using the
///        accessors.
/// @param obj       Object to serialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
/// @return Function (void(auto bytes, size_t pos = 0)) that serialize the
///         attributes accessible via `accessors`
template <typename Ser>
constexpr inline auto bindSerialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        return serialize<Ser>(mem, pos, std::invoke(accessors, obj)...);
    };
}

/// @brief Create a function for deserializing the given object using the
///        accessors.
/// @param obj       Object to deserialize.
/// @param accessors Accessors to the attributes (or the attributes themselves).
/// @return Function (void(auto bytes, size_t pos = 0)) that deserialize the
///         attributes accessible via `accessors`
template <typename Ser>
constexpr inline auto bindDeserialize(auto &obj, auto &&...accessors) {
    return [=, &obj](auto &mem, size_t pos = 0) {
        return tools::deserializerAccessors<Ser>(mem, pos, obj, accessors...);
    };
}

} // end namespace serializer

#endif
