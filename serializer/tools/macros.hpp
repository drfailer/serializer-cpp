#ifndef SERIALIZER_MACROS_H
#define SERIALIZER_MACROS_H

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
        return serializer::serializeWithId<Ser, decltype(this)>(mem, pos,      \
                                                                __VA_ARGS__);  \
    }                                                                          \
    constexpr virt size_t deserialize(MemT &mem, size_t pos = 0) over {        \
        return serializer::deserializeWithId<Ser, decltype(this)>(             \
            mem, pos, __VA_ARGS__);                                            \
    }

/// @brief Generate the serialze and deserialize methods with the specified
///        serializer.
/// @param Ser Serializer.
/// @param ... Members to serialize.
#define SERIALIZE_CUSTOM(Ser, ...)                                             \
    __SERIALIZE__(Ser, auto, /* virt */, /* over */, __VA_ARGS__)

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
#define SERIALIZE_EMPTY(Ser)                                                   \
    constexpr virtual size_t serialize(typename Ser::mem_type &, size_t = 0)   \
        const {                                                                \
        return 0;                                                              \
    }                                                                          \
    constexpr virtual size_t deserialize(typename Ser::mem_type &,             \
                                         size_t = 0) {                         \
        return 0;                                                              \
    }

/// @brief Generate abstract serialize and deserialize virtual methods.
/// @param MemT Type of the buffer (virtual methods cannot be template).
#define SERIALIZE_ABSTRACT(Ser)                                                \
    virtual size_t serialize(typename Ser::mem_type &, size_t = 0) const = 0;  \
    virtual size_t deserialize(typename Ser::mem_type &, size_t = 0) = 0;

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
/*                               helper macros                                */
/******************************************************************************/

/// @brief Helper macro that can be used to avoid writing all the namespaces.
/// @param ArrType Type of the pointer to serialize (int* for a basic array).
/// @param ... Types of the dimensions (ex: size_t if we want to pass it by
///            value or size_t& by reference)
#define SER_DARR(...) serializer::tools::DynamicArray(__VA_ARGS__)

/// @brief Helper macro for SERIALIZE_CUSTOM (get the type of the bytes buffer)
#define SER_MEMT decltype(mem)

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
