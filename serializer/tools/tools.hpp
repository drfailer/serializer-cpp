#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../meta/concepts.hpp"

namespace serializer::tools {

/*
 * The convertor contains serialize and deserialize template functions that are
 * used for serializing various types. Here, we use concepts to generate the
 * right implementations for the different types.
 *
 * Implementations of the functions are defined in a macro to allow the user to
 * create its own Convertor class.
 */

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

/// @brief Insert an element into an iterable using the insert member function.
template <typename Container, typename T>
    requires serializer::concepts::Insertable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.insert(element);
}

/// @brief Insert an element into an iterable using the add member
///        function.
template <typename Container, typename T>
    requires serializer::concepts::PushBackable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.push_back(element);
}

/// @brief Insert an element into an iterable using the operator[].
template <typename Container, typename T>
inline constexpr void insert(Container &&container, T &&element, size_t idx) {
    container[idx] = element;
}

/******************************************************************************/
/*                             tuple manipulation                             */
/******************************************************************************/

template <typename H, typename... Types, size_t... Idx>
std::tuple<Types...> tuplePopFront_(std::tuple<H, Types...> const &t,
                                    std::index_sequence<Idx...>) {
    return std::tuple<Types...>(std::get<Idx + 1>(t)...);
}

template <typename H, typename... Types>
std::tuple<Types...> tuplePopFront(std::tuple<H, Types...> const &t) {
    return tuplePopFront_(t, std::make_index_sequence<sizeof...(Types)>());
}

template <typename T, typename... Types, size_t... Idx>
T tupleProd_(std::tuple<Types...> const &tuple, std::index_sequence<Idx...>) {
    if constexpr (sizeof...(Types) == 0) {
        return 0;
    } else {
        return (std::get<Idx>(tuple) * ...);
    }
}

template <typename T, typename... Types>
T tupleProd(std::tuple<Types...> const &tuple) {
    return tupleProd_<T>(tuple, std::make_index_sequence<sizeof...(Types)>());
}

/******************************************************************************/
/*                               helper macros                                */
/******************************************************************************/

/// @brief Macro that allow to get the name_ of a type using the RTTI.
#define class_name(Type) typeid(Type).name()

} // namespace serializer::tools

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/// @brief Used to simplify the writing of a custom convertor for polymorphic
///        types (allow to add support for all common pointers)
#define POLYMORPHIC_TYPE(Class)                                                \
    Class *, std::shared_ptr<Class>, std::unique_ptr<Class>

/// @brief Generates a deserialize function for the polymorphic type
///        GenericType.
///        It is outside of the tools namespace to avoid errors when using it.
#define HANDLE_POLYMORPHIC_IMPL(IdTable, GenericType, ...)                     \
    constexpr void serialize(GenericType const &elt) override {                \
        this->pos = elt->serialize(this->mem, this->pos);                      \
    }                                                                          \
    constexpr void deserialize(GenericType &elt) override {                    \
        auto id = this->template getId<IdTable::id_type>();                    \
        serializer::tools::createGeneric(id, IdTable(), elt);                  \
        this->pos = elt->deserialize(this->mem, this->pos);                    \
    }

/// @brief Generates a deserialize function for the pointers and smart pointers
///        of the polymorphic type GenericType.
///        It is outside of the tools namespace to avoid errors when using it.
#define HANDLE_POLYMORPHIC(IdTable, GenericType, ...)                          \
    HANDLE_POLYMORPHIC_IMPL(IdTable, GenericType *, __VA_ARGS__)               \
    HANDLE_POLYMORPHIC_IMPL(IdTable, std::shared_ptr<GenericType>,             \
                            __VA_ARGS__)                                       \
    HANDLE_POLYMORPHIC_IMPL(IdTable, std::unique_ptr<GenericType>, __VA_ARGS__)

/// @brief Helper macro to create a lambdat that is executed during the
///        serialization and the deserialization.
/// @param code Code to execute.
#define SER_FUN(code)                                                          \
    [&]<serializer::tools::Phases Phase, typename Conv>(                       \
        [[maybe_unused]] serializer::tools::Context<Phase, Conv> &&context)    \
        code

/// @brief Helper macro to create a lambdat that is executed during the
///        serialization.
/// @param code Code to execute.
#define SER_SFUN(code)                                                         \
    SER_FUN({                                                                  \
        if constexpr (Phase == serializer::tools::Phases::Serialization)       \
            code;                                                              \
    })

/// @brief Helper macro to create a lambdat that is executed during the
///        deserialization.
/// @param code Code to execute.
#define SER_DFUN(code)                                                         \
    SER_FUN({                                                                  \
        if constexpr (Phase == serializer::tools::Phases::Deserialization)     \
            code;                                                              \
    })

#endif
