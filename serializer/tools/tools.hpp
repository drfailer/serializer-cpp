#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../exceptions/unknown_specialized_type.hpp"
#include "concepts.hpp"

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
    requires serializer::tools::concepts::Insertable<Container, T>
void insert(Container &container, T &&element) {
    if constexpr (concepts::Forwardable<T>) {
        container.insert(std::forward<T>(element));
    } else {
        container.insert(element);
    }
}

/// @brief Insert an element into an iterable using the add member
///        function.
template <typename Container, typename T>
    requires serializer::tools::concepts::PushBackable<Container, T>
void insert(Container &container, T &&element) {
    if constexpr (concepts::Forwardable<T>) {
        container.push_back(std::forward<T>(element));
    } else {
        container.push_back(element);
    }
}

/// @brief Insert an element into an iterable using the operator[].
template <typename Container, typename T>
void insert(Container &container, T &&element, size_t idx) {
    if constexpr (concepts::Forwardable<T>) {
        container[idx] = std::forward<T>(element);
    } else {
        container[idx] = element;
    }
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

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/// @brief Function that is used to deserialize polymorphic types. It iterates
///        on the types (given threw template parameters) and create the right
///        object. All the types should derive RT.
///        This overload is used to end the template recursion. In this case, we
///        don't know the type so we throw an exception.
/// @param className Name of the class obtained using typeid.
/// @param str View of the string to deserialize.
template <typename RT>
RT type_switch_fn(const std::string_view &className, std::string_view &) {
    throw serializer::exceptions::UknownSpecializedTypeError<RT>(className);
}

/// @brief Function that is used to deserialize polymorphic types. It iterates
///        on the types (given threw template parameters) and create the right
///        object. All the types should derive RT.
/// @param className Name of the class obtained using typeid.
/// @param str View of the string to deserialize.
template <typename RT, typename T, typename... Types>
RT type_switch_fn(const std::string_view &className, std::string_view &str) {
    static_assert(!std::is_abstract_v<T>, "The type shouldn't be abstract.");
    static_assert(std::is_default_constructible_v<T>,
                  "The pointer types should be default constructible.");
    if (className == class_name(T)) {
        if constexpr (mtf::is_shared_v<RT>) {
            auto elt = std::make_shared<T>();
            elt->deserialize(str);
            return elt;
        } else if constexpr (mtf::is_unique_v<RT>) {
            auto elt = std::make_unique<T>();
            elt->deserialize(str);
            return elt;
        } else {
            T *elt = new T();
            elt->deserialize(str);
            return elt;
        }
    }
    return type_switch_fn<RT, Types...>(className, str);
}

} // namespace serializer::tools

/// @brief Used to simplify the writing of a custom convertor for polymorphic
///        types (allow to add support for all common pointers)
#define POLYMORPHIC_TYPE(Class)                                                \
    Class *, std::shared_ptr<Class>, std::unique_ptr<Class>

/// @brief Generates a deserialize function for the polymorphic type
///        GenericType.
///        It is outside of the tools namespace to avoid errors when using it.
#define HANDLE_POLYMORPHIC_IMPL(GenericType, ...)                              \
    GenericType deserialize(std::string_view &str, GenericType &) override {   \
        bool ptrValid = str[0] == 'v';                                         \
        str = str.substr(1);                                                   \
                                                                               \
        if (!ptrValid) {                                                       \
            return nullptr;                                                    \
        }                                                                      \
        using size_type = typename std::string::size_type;                     \
        size_type size = *reinterpret_cast<const size_type *>(str.data());     \
        std::string_view className = str.substr(sizeof(size), size);           \
        return serializer::tools::type_switch_fn<GenericType, __VA_ARGS__>(    \
            className, str);                                                   \
    }                                                                          \
    void serialize(GenericType const &elt, std::string &str) const override {  \
        Convertor::serialize_(elt, str);                                       \
    }

/// @brief Generates a deserialize function for the pointers and smart pointers
///        of the polymorphic type GenericType.
///        It is outside of the tools namespace to avoid errors when using it.
#define HANDLE_POLYMORPHIC(GenericType, ...)                                   \
    HANDLE_POLYMORPHIC_IMPL(GenericType *, __VA_ARGS__)                        \
    HANDLE_POLYMORPHIC_IMPL(std::shared_ptr<GenericType>, __VA_ARGS__)         \
    HANDLE_POLYMORPHIC_IMPL(std::unique_ptr<GenericType>, __VA_ARGS__)

/// @brief Helper macro to create a lambdat that is executed during the
///        serialization and the deserialization.
/// @param code Code to execute.
#define SER_FUN(code)                                                          \
    [&]([[maybe_unused]] serializer::Phases phase,                             \
        [[maybe_unused]] std::string_view const &str) { code; }

/// @brief Helper macro to create a lambdat that is executed during the
///        serialization.
/// @param code Code to execute.
#define SER_SFUN(code)                                                         \
    [&](serializer::Phases phase,                                              \
        [[maybe_unused]] std::string_view const &str) {                        \
        if (phase == serializer::Phases::Serialization) {                      \
            code;                                                              \
        }                                                                      \
    }

/// @brief Helper macro to create a lambdat that is executed during the
///        deserialization.
/// @param code Code to execute.
#define SER_DFUN(code)                                                         \
    [&](serializer::Phases phase, [[maybe_unused]] std::string_view const &) { \
        if (phase == serializer::Phases::Deserialization) {                    \
            code;                                                              \
        }                                                                      \
    }

#endif
