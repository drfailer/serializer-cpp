#ifndef UTILS_HPP
#define UTILS_HPP
#include "concepts.hpp"

namespace serializer::utility {

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

// sorted container
template <typename Container, typename T>
    requires serializer::concepts::Insertable<Container, T>
void insert(Container &container, const T &element) {
    container.insert(element);
}

// sequence container
template <typename Container, typename T>
    requires serializer::concepts::PushBackable<Container, T>
void insert(Container &container, const T &element) {
    container.push_back(element);
}

// arrays
template <typename Container, typename T>
void insert(Container &container, const T &element, size_t idx) {
    container[idx] = element;
}

/******************************************************************************/
/*                               helper macros                                */
/******************************************************************************/

#define class_name(Type) typeid(Type).name()

/******************************************************************************/
/*                          deserialize polymorphic                           */
/******************************************************************************/

/*
 * This macro will generate a deserialize function for polymorphic types. The
 * purpose is to help the user to easily create its own convertor that is
 * capable of handling polymorphic types.
 *
 * Here we juste have to use the macro DESERIALIZE_POLYMORPHIC. It takes as
 * arguments the super class and the list of classes that inherits from this
 * super class. The resulting function will return a heap allocated pointer of
 * type Super class that is created using the right constructor.
 */

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

} // namespace serializer::utility

/*
 * Generates a deserialize function for the polymorphic type GenericType.
 */
#define HANDLE_POLYMORPHIC(GenericType, ...)                                   \
    GenericType *deserialize_(std::string_view &str, GenericType *&)           \
        override {                                                             \
        using size_type = typename std::string::size_type;                     \
        size_type size = *reinterpret_cast<const size_type *>(str.data());     \
        std::string_view className = str.substr(sizeof(size), size);           \
        return serializer::utility::type_switch_fn<GenericType *,              \
                                                   __VA_ARGS__>(className,     \
                                                                str);          \
    }                                                                          \
    std::string &serialize_(GenericType *const &elt, std::string &str)         \
        const override {                                                       \
        return Convertor::serialize(elt, str);                             \
    }

#endif
