#ifndef SERIALIZER_TOOLS_HPP
#define SERIALIZER_TOOLS_HPP
#include "../meta/concepts.hpp"

namespace serializer::tools {

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

/// @brief Insert an element into an iterable using the insert member function.
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
template <typename Container, typename T>
    requires serializer::concepts::Insertable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.insert(element);
}

/// @brief Insert an element into an iterable using the add member
///        function.
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
template <typename Container, typename T>
    requires serializer::concepts::PushBackable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.push_back(element);
}

/// @brief Insert an element into an iterable using the operator[].
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
/// @param idx Index where the element should be inserted in the container.
template <typename Container, typename T>
inline constexpr void insert(Container &&container, T &&element, size_t idx) {
    container[idx] = element;
}

/******************************************************************************/
/*                             tuple manipulation                             */
/******************************************************************************/

/// @brief Remove the first element of a tuple.
/// @tparam H First type of the tuple.
/// @tparam Tyes Other types in the tuple.
/// @tparam Idx  Indicies in the tuple.
/// @param t Tuple.
/// @param index_sequence Indicies of the elements of the tuple.
/// @return Tail of the tuple.
template <typename H, typename... Types, size_t... Idx>
std::tuple<Types...> tuplePopFront_(std::tuple<H, Types...> const &tuple,
                                    std::index_sequence<Idx...>) {
    return std::tuple<Types...>(std::get<Idx + 1>(tuple)...);
}

/// @brief Remove the first element of a tuple.
/// @tparam H First type of the tuple.
/// @tparam Tyes Other types in the tuple.
/// @param t Tuple.
/// @return Tail of the tuple.
template <typename H, typename... Types>
std::tuple<Types...> tuplePopFront(std::tuple<H, Types...> const &tuple) {
    return tuplePopFront_(tuple, std::make_index_sequence<sizeof...(Types)>());
}

/// @brief Multiply the elements in a tuple.
/// @tparam T Output type.
/// @tparam Types Types in the tuple.
/// @tparma Idx Index sequence
/// @param tuple Tuple.
/// @param index_sequence
template <typename T, typename... Types, size_t... Idx>
T tupleProd_(std::tuple<Types...> const &tuple, std::index_sequence<Idx...>) {
    if constexpr (sizeof...(Types) == 0) {
        return 0;
    } else {
        return (std::get<Idx>(tuple) * ...);
    }
}

/// @brief Multiply the elements in a tuple.
/// @tparam T Output type.
/// @tparam Types Types in the tuple.
/// @param tuple Tuple.
template <typename T, typename... Types>
T tupleProd(std::tuple<Types...> const &tuple) {
    return tupleProd_<T>(tuple, std::make_index_sequence<sizeof...(Types)>());
}

} // namespace serializer::tools

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/// @brief Used to simplify the writing of a custom convertor for polymorphic
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

/// @brief Helper macro to create a lambda that is executed during the
///        serialization and the deserialization.
/// @param code Code to execute.
#define SER_FUN(code)                                                          \
    [&]<serializer::tools::Phases Phase, typename Conv>(                       \
        [[maybe_unused]] serializer::tools::Context<Phase, Conv> &&context)    \
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
