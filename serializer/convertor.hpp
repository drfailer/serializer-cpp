#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "metafunctions.hpp"
#include "parser.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

/*
 * The convertor contains serialize and deserialize template functions that are
 * used for serializing various types. Here, we use SFINAE to generate the right
 * implementations for the different types.
 *
 * Implementations of the functions are defined in a macro to allow the user to
 * create its own Convertor class.
 */

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

// sorted container
template <
    typename Container, typename T,
    decltype(std::declval<Container>().insert(std::declval<T>())) * = nullptr>
void insert(Container &container, const T &element) {
    container.insert(element);
}

// sequence container
template <typename Container, typename T,
          decltype(std::declval<Container>().push_back(std::declval<T>())) * =
              nullptr>
void insert(Container &container, const T &element) {
    container.push_back(element);
}

/******************************************************************************/
/*                      default convertor implementation                      */
/******************************************************************************/

/* I'm sorry for this :,(
 *
 * We want the user to be able to define it's own convertor to implement missing
 * convertion functions. The ideal solution would use inheritance but this
 * cannot work as the Convertor member functions cannot call the functions of
 * the derived class. So the first time the custom convertor calls one of the
 * functions in the default one, we cannot use custom functions anymore as they
 * are not visible for the default convertor.
 *
 * We could find a way for the default convertor and the custom one to know each
 * other and allow function calls between theme. However, the solution would be
 * even more complex and it would be really difficult to make it easy to use.
 *
 * This macro looks terrible and it makes the library annoying to maintain but
 * this is the best solution I have for know.
 */
#define CONVERTOR                                                              \
    /* deserialize ********************************************************/   \
                                                                               \
    template <typename T,                                                      \
              decltype(std::declval<T>().deserialize("")) * = nullptr>         \
    static std::remove_reference_t<T> deserialize(const std::string &str) {    \
        std::remove_reference_t<T> t;                                          \
        t.deserialize(str);                                                    \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename T,                                                      \
              std::enable_if_t<std::is_fundamental_v<T>> * = nullptr>          \
    static T deserialize(const std::string &str) {                             \
        T t;                                                                   \
        std::istringstream(str) >> t;                                          \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<is_concrete_ptr<T>> * = nullptr>    \
    static std::remove_reference_t<T> deserialize(const std::string &str) {    \
        if (str == "nullptr") {                                                \
            return nullptr;                                                    \
        }                                                                      \
        using Type =                                                           \
            typename std::remove_pointer_t<std::remove_reference_t<T>>;        \
        Type *t = new Type();                                                  \
                                                                               \
        if constexpr (std::is_fundamental_v<Type>) {                           \
            *t = deserialize<Type>(str);                                       \
        } else {                                                               \
            t->deserialize(str);                                               \
        }                                                                      \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename SP, std::enable_if_t<is_smart_ptr_v<SP>> * = nullptr>   \
    static SP deserialize(const std::string &str) {                            \
        if (str == "nullptr") {                                                \
            return nullptr;                                                    \
        }                                                                      \
        SP t;                                                                  \
        if constexpr (is_shared_v<SP>) {                                       \
            t = std::make_shared<typename SP::element_type>();                 \
        } else if constexpr (is_unique_v<SP>) {                                \
            t = std::make_unique<typename SP::element_type>();                 \
        }                                                                      \
                                                                               \
        if constexpr (std::is_fundamental_v<typename SP::element_type>) {      \
            *t = deserialize<typename SP::element_type>(str);                  \
        } else {                                                               \
            t->deserialize(str);                                               \
        }                                                                      \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<is_pair_v<T>> * = nullptr>          \
    static T deserialize(const std::string &str) {                             \
        using first_type = typename T::first_type;                             \
        using second_type = typename T::second_type;                           \
        std::pair<std::string, std::string> content = parsePair(str);          \
        first_type elt1 =                                                      \
            deserialize<std::remove_const_t<first_type>>(content.first);       \
        second_type elt2 =                                                     \
            deserialize<std::remove_const_t<second_type>>(content.second);     \
        return T(elt1, elt2);                                                  \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<std::is_enum_v<T>> * = nullptr>     \
    static T deserialize(const std::string &str) {                             \
        std::istringstream iss(str);                                           \
        std::underlying_type_t<T> out;                                         \
        iss >> out;                                                            \
        return (T)out;                                                         \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<is_string_v<T>> * = nullptr>        \
    static std::string deserialize(const std::string &str) {                   \
        std::string t = str.substr(1, str.size() - 2);                         \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename T, typename T::iterator * = nullptr,                    \
              std::enable_if_t<!is_string_v<T>> * =                            \
                  nullptr, /* we have to make sure that the iterable value is  \
                              serializable */                                  \
              decltype(deserialize<iter_value_t<T>>("")) * = nullptr>          \
    static T deserialize(const std::string &str) {                             \
        T result;                                                              \
        std::size_t valueStart = 2;                                            \
        std::size_t valueEnd;                                                  \
                                                                               \
        while (valueStart < str.size()) {                                      \
            valueEnd = findEndValueIndex(str, valueStart);                     \
            insert(result, deserialize<iter_value_t<T>>(str.substr(            \
                               valueStart, valueEnd - valueStart)));           \
            valueStart = valueEnd + 2; /* value1, value2 */                    \
        }                                                                      \
                                                                               \
        return result;                                                         \
    }                                                                          \
                                                                               \
    /* serialize ***********************************************************/  \
                                                                               \
    template <typename T, decltype(std::declval<T>().serialize()) * = nullptr> \
    static std::string serialize(T &elt) {                                     \
        return elt.serialize();                                                \
    }                                                                          \
                                                                               \
    template <typename T,                                                      \
              std::enable_if_t<std::is_fundamental_v<T>> * = nullptr>          \
    static std::string serialize(T &elt) {                                     \
        std::ostringstream oss;                                                \
        oss << elt;                                                            \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<std::is_pointer_v<T>> * = nullptr>  \
    static std::string serialize(T elt) {                                      \
        if (elt != nullptr) {                                                  \
            if constexpr (std::is_abstract_v<std::remove_pointer_t<T>>) {      \
                return elt->serialize();                                       \
            } else {                                                           \
                return serialize<std::remove_pointer_t<T>>(*elt);              \
            }                                                                  \
        } else {                                                               \
            return "nullptr";                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <typename SP, std::enable_if_t<is_smart_ptr_v<SP>> * = nullptr>   \
    static std::string serialize(SP &elt) {                                    \
        if (elt != nullptr) {                                                  \
            if constexpr (std::is_abstract_v<typename SP::element_type>) {     \
                return elt->serialize();                                       \
            } else {                                                           \
                return serialize<typename SP::element_type>(*elt);             \
            }                                                                  \
        } else {                                                               \
            return "nullptr";                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<is_pair_v<T>> * = nullptr>          \
    static std::string serialize(const T &elt) {                               \
        std::ostringstream oss;                                                \
        oss << "{ " << serialize(elt.first) << ", " << serialize(elt.second)   \
            << " }";                                                           \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<std::is_enum_v<T>> * = nullptr>     \
    static std::string serialize(const T &elt) {                               \
        std::ostringstream oss;                                                \
        std::underlying_type_t<T> v = (std::underlying_type_t<T>)elt;          \
        oss << v;                                                              \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    static inline std::string serialize(const std::string &elt) {              \
        std::ostringstream oss;                                                \
        oss << "\"" << elt << "\"";                                            \
        return oss.str();                                                      \
    }                                                                          \
                                                                               \
    template <typename T, typename T::iterator * = nullptr,                    \
              std::enable_if_t<!is_string_v<T>> * =                            \
                  nullptr, /* we have to make sure that the iterable value is  \
                              serializable */                                  \
              decltype(serialize(std::declval<const iter_value_t<T> &>())) * = \
                  nullptr>                                                     \
    static std::string serialize(const T &elts) {                              \
        std::ostringstream oss;                                                \
        auto it = elts.begin();                                                \
                                                                               \
        if (it != elts.end()) {                                                \
            oss << "[ " << serialize(*it++);                                   \
            for (; it != elts.end(); it++) {                                   \
                oss << ", " << serialize(*it);                                 \
            }                                                                  \
            oss << " ]";                                                       \
        }                                                                      \
        return oss.str();                                                      \
    }

/******************************************************************************/
/*                          default convertor class                           */
/******************************************************************************/

struct Convertor {
    CONVERTOR;
};

/******************************************************************************/
/*                               helper macros                                */
/******************************************************************************/

/* facilitate the creation of a custom deserialize function for a specific type.
 *
 * Example:
 * serialize_custom_type(MyType, const std::string str) {
 *     ...
 * }
 */
#define deserialize_custom_type(Type, input)                                   \
    template <typename T,                                                      \
              std::enable_if_t<std::is_same_v<base_t<T>, Type>> * = nullptr>   \
    static Type deserialize(input)

#define serialize_custom_type(input) static std::string serialize(input)

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
 * arguemnts the super class and the list of classes that inherits from this
 * super class. The resulting function will return a heap allocated pointer of
 * type Super class that is created using the right constructor.
 */

template <typename RT>
RT type_switch_fn(const std::string &, const std::string &) {
    return nullptr;
}

template <typename RT, typename T, typename... Types>
RT type_switch_fn(const std::string &className, const std::string &str) {
    static_assert(!std::is_abstract_v<T>, "The type shouldn't be abstract.");
    if (className == class_name(T)) {
        T *elt = new T();
        elt->deserialize(str);
        return elt;
    }
    return type_switch_fn<RT, Types...>(className, str);
}

/*
 * Generates a deserialize function for the polymorphic type GenericType.
 */
#define DESERIALIZE_POLYMORPHIC(GenericType, ...)                              \
    template <typename T,                                                      \
              std::enable_if_t<std::is_same_v<base_t<T>, GenericType *>> * =   \
                  nullptr>                                                     \
    static GenericType *deserialize(const std::string &str) {                  \
        return type_switch_fn<GenericType *, __VA_ARGS__>(                     \
            getThisClassName(str), str);                                       \
    }

#endif
