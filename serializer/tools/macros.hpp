#ifndef SERIALIZER_MACROS_H
#define SERIALIZER_MACROS_H
#include "cstddef"
#include "type_table.hpp"

/// @brief namespace serializer macros
namespace serializer::macros {

/// @brief Serialize the members and the id if required.
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

/// @brief Deserialize the members and the id if required.
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
} // end namespace serializer::macros

/******************************************************************************/
/*                                 serialize                                  */
/******************************************************************************/

/// @brief Generate the serialize and deserialize methods with the specified
///        serializer. The keywords virtual and override can be added.
/// @param Ser Serializer.
/// @param virt Virtual keyworkd.
/// @param over Override keyworkd.
/// @param ... Members to serialize.
#define __SERIALIZE__(Ser, MemT, virt, over, ...)                              \
    constexpr virt size_t serialize(MemT &mem, size_t pos = 0) const over {    \
        return serializer::macros::serializeWithId<Ser, decltype(this)>(       \
            mem, pos, __VA_ARGS__);                                            \
    }                                                                          \
    constexpr virt size_t deserialize(MemT &mem, size_t pos = 0) over {        \
        return serializer::macros::deserializeWithId<Ser, decltype(this)>(     \
            mem, pos, __VA_ARGS__);                                            \
    }

/// @brief Generate the serialze and deserialize methods with the specified
///        serializer.
/// @param Ser Serializer.
/// @param ... Members to serialize.
#define SERIALIZE_CONV(Ser, ...)                                               \
    __SERIALIZE__(Ser, typename Ser::mem_type, /* virt */, /* over */,         \
                  __VA_ARGS__)

/// @brief Generate the serialize and deserialize methods with the default
///        serializer.
/// @param ... Members to serialize.
#define SERIALIZE(...)                                                         \
    __SERIALIZE__(serializer::Serializer<decltype(mem)>, auto, /* virt */,     \
                  /* over */, __VA_ARGS__)

/// @brief Generate the serialze and deserialize virtual methods using the
///        specified serializer.
/// @param Ser Serializer
/// @param ... Members to serialize
#define VIRTUAL_SERIALIZE(Ser, ...)                                            \
    __SERIALIZE__(Ser, typename Ser::mem_type, virtual, /* over */, __VA_ARGS__)

/// @brief Generate the serialze and deserialize override methods using the
///        specified serializer.
/// @param Ser Serializer
/// @param ... Members to serialize
#define SERIALIZE_OVERRIDE(Ser, ...)                                           \
    __SERIALIZE__(Ser, typename Ser::mem_type, /* virt */, override,           \
                  __VA_ARGS__)

/// @brief Generate empty serialize and deserialize virtual methods (useful
///        whithin a concrete super class).
/// @param MemT Type of the buffer (virtual methods cannot be template).
#define SERIALIZE_EMPTY(MemT)                                                  \
    constexpr virtual size_t serialize(MemT &, size_t = 0) const { return 0; } \
    constexpr virtual size_t deserialize(MemT &, size_t = 0) { return 0; }

/// @brief Generate abstract serialize and deserialize virtual methods.
/// @param MemT Type of the buffer (virtual methods cannot be template).
#define SERIALIZE_ABSTRACT(Ser)                                                \
    constexpr virtual size_t serialize(typename Ser::mem_type &, size_t = 0)   \
        const = 0;                                                             \
    constexpr virtual size_t deserialize(typename Ser::mem_type &,             \
                                         size_t = 0) = 0;

/// @brief Generate the serialize and deserialize methods that use the
///        (de)serialize functions.
#define SERIALIZE_STRUCT()                                                     \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serializeStruct(mem, pos, this);                    \
    }                                                                          \
    constexpr size_t deserialize(auto const &mem, size_t pos = 0) {            \
        return serializer::deserializeStruct(mem, pos, this);                  \
    }

/******************************************************************************/
/*                            serializer functions                            */
/******************************************************************************/

/// @brief Helper macro to create a lambda that is executed during the
///        serialization and the deserialization.
/// @param code Code to execute.
#define SER_FUN(code)                                                          \
    [&]<serializer::tools::Phases Phase, typename Ser>(                        \
        [[maybe_unused]] serializer::tools::Context<Phase, Ser> &&context)     \
        code

/// @brief Helper macro to create a lambda that is executed during the
///        serialization.
/// @param code Code to execute.
#define SER_SFUN(code)                                                         \
    SER_FUN({                                                                  \
        if constexpr (Phase == serializer::tools::Phases::Serialization)       \
            code;                                                              \
    })

/// @brief Helper macro to create a lambda that is executed during the
///        deserialization.
/// @param code Code to execute.
#define SER_DFUN(code)                                                         \
    SER_FUN({                                                                  \
        if constexpr (Phase == serializer::tools::Phases::Deserialization)     \
            code;                                                              \
    })

#endif
