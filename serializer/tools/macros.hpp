#ifndef SERIALIZER_MACROS_H
#define SERIALIZER_MACROS_H
#include "../serialize.hpp"

/******************************************************************************/
/*                                 serialize                                  */
/******************************************************************************/

/// @brief Generate the serialze and deserialize methods with the specified
///        serializer.
/// @param Ser Serializer.
/// @param ... Members to serialize.
#define SERIALIZE_CONV(Ser, ...)                                               \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize<Ser<decltype(mem)>>(mem, pos,             \
                                                         __VA_ARGS__);         \
    }                                                                          \
    constexpr size_t deserialize(auto &mem, size_t pos = 0) {                  \
        return serializer::deserialize<Ser<decltype(mem)>>(mem, pos,           \
                                                           __VA_ARGS__);       \
    }

/// @brief Generate the serialize and deserialize methods with the default
///        serializer.
/// @param ... Members to serialize.
#define SERIALIZE(...) SERIALIZE_CONV(serializer::Serializer, __VA_ARGS__)

/// @brief Generate the serialize and deserialize methods with the specified
///        serializer. The keywords virtual and override can be added.
/// @param Ser Serializer.
/// @param virt Virtual keyworkd.
/// @param over Override keyworkd.
/// @param ... Members to serialize.
#define __SERIALIZE__(Ser, virt, over, ...)                                    \
    constexpr virt size_t serialize(typename Ser::mem_type &mem,               \
                                    size_t pos = 0) const over {               \
        return serializer::serialize<Ser>(mem, pos, __VA_ARGS__);              \
    }                                                                          \
    constexpr virt size_t deserialize(typename Ser::mem_type &mem,             \
                                      size_t pos = 0) over {                   \
        return serializer::deserialize<Ser>(mem, pos, __VA_ARGS__);            \
    }

/// @brief Generate the serialze and deserialize virtual methods using the
///        specified serializer.
/// @param Ser Serializer
/// @param ... Members to serialize
#define VIRTUAL_SERIALIZE(Ser, ...)                                            \
    __SERIALIZE__(Ser, virtual, /* over */, __VA_ARGS__)

/// @brief Generate the serialze and deserialize override methods using the
///        specified serializer.
/// @param Ser Serializer
/// @param ... Members to serialize
#define SERIALIZE_OVERRIDE(Ser, ...)                                           \
    __SERIALIZE__(Ser, /* virt */, override, __VA_ARGS__)

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

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/// @brief Used to simplify the writing of a custom serializer for polymorphic
///        types (allow to add support for all common pointers)
#define POLYMORPHIC_TYPE(Class)                                                \
    Class *, std::shared_ptr<Class>, std::unique_ptr<Class>

/// @brief Generates serialize and deserialize functions for the polymorphic
///        type GenericType.
/// @param TypeTable Table to have the identifiers of the serialized types.
/// @param GenericType Type of the super class.
/// @param ... Subtypes.
#define HANDLE_POLYMORPHIC_IMPL(TypeTable, GenericType, ...)                   \
    constexpr void serialize(GenericType const &elt) override {                \
        this->pos = elt->serialize(this->mem, this->pos);                      \
    }                                                                          \
    constexpr void deserialize(GenericType &elt) override {                    \
        auto id = serializer::tools::getId<TypeTable::id_type>(this->mem,      \
                                                               this->pos);     \
        serializer::tools::createGeneric(id, TypeTable(), elt);                \
        this->pos = elt->deserialize(this->mem, this->pos);                    \
    }

/// @brief Generates a deserialize function for the pointers and smart pointers
///        of the polymorphic type GenericType.
/// @param TypeTable Table to have the identifiers of the serialized types.
/// @param GenericType Type of the super class.
/// @param ... Subtypes.
#define HANDLE_POLYMORPHIC(TypeTable, GenericType, ...)                        \
    HANDLE_POLYMORPHIC_IMPL(TypeTable, GenericType *, __VA_ARGS__)             \
    HANDLE_POLYMORPHIC_IMPL(TypeTable, std::shared_ptr<GenericType>,           \
                            __VA_ARGS__)                                       \
    HANDLE_POLYMORPHIC_IMPL(TypeTable, std::unique_ptr<GenericType>,           \
                            __VA_ARGS__)

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
