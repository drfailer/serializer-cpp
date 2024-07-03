#ifndef UTILS_HPP
#define UTILS_HPP
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
void insert(Container &container, const T &element) {
    container.insert(element);
}

/// @brief Insert an element into an iterable using the push_back member
///        function.
template <typename Container, typename T>
    requires serializer::concepts::PushBackable<Container, T>
void insert(Container &container, const T &element) {
    container.push_back(element);
}

/// @brief Insert an element into an iterable using the operator[].
template <typename Container, typename T>
void insert(Container &container, const T &element, size_t idx) {
    container[idx] = element;
}

/******************************************************************************/
/*                               helper macros                                */
/******************************************************************************/

/// @brief Macro that allow to get the name of a type using the RTTI.
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
    if (className == class_name(T)) {
        T *elt = new T();
        elt->deserialize(str);
        return elt;
    }
    return type_switch_fn<RT, Types...>(className, str);
}

} // namespace serializer::tools

/// @brief Generates a deserialize function for the polymorphic type
///        GenericType. It is outside of the tools namespace to avoid errors
///        when using it.
#define HANDLE_POLYMORPHIC(GenericType, ...)                                   \
    GenericType *deserialize_(std::string_view &str, GenericType *&)           \
        override {                                                             \
        using size_type = typename std::string::size_type;                     \
        size_type size = *reinterpret_cast<const size_type *>(str.data());     \
        std::string_view className = str.substr(sizeof(size), size);           \
        return serializer::tools::type_switch_fn<GenericType *, __VA_ARGS__>(  \
            className, str);                                                   \
    }                                                                          \
    std::string &serialize_(GenericType *const &elt, std::string &str)         \
        const override {                                                       \
        return Convertor::serialize(elt, str);                                 \
    }

#endif
