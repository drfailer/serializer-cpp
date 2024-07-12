#ifndef TOOLS_HPP
#define TOOLS_HPP
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
    requires serializer::concepts::Insertable<Container, T>
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
    requires serializer::concepts::PushBackable<Container, T>
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
/*                               helper macros                                */
/******************************************************************************/

/// @brief Macro that allow to get the name_ of a type using the RTTI.
#define class_name(Type) typeid(Type).name()

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/// @brief This macro will generate a deserialize function for polymorphic
///        types. The purpose is to help the user to easily create its own
///        convertor that is capable of handling polymorphic types.
///
///        Here we juste have to use the macro DESERIALIZE_POLYMORPHIC. It takes
///        as arguments the super class and the list of classes that inherits
///        from this super class. The resulting function will return a heap
///        allocated pointer of type Super class that is created using the right
///        constructor.

template <typename RT>
RT type_switch_fn(const std::string_view &, std::string_view &) {
    return nullptr;
}

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
    std::string &serialize(GenericType const &elt, std::string &str)           \
        const override {                                                       \
        return Convertor::serialize_(elt, str);                                \
    }

/// @brief Generates a deserialize function for the pointers and smart pointers
///        of the polymorphic type GenericType.
///        It is outside of the tools namespace to avoid errors when using it.
#define HANDLE_POLYMORPHIC(GenericType, ...)                                   \
    HANDLE_POLYMORPHIC_IMPL(GenericType *, __VA_ARGS__)                        \
    HANDLE_POLYMORPHIC_IMPL(std::shared_ptr<GenericType>, __VA_ARGS__)         \
    HANDLE_POLYMORPHIC_IMPL(std::unique_ptr<GenericType>, __VA_ARGS__)

#endif
