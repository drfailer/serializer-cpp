#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "parser.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>

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
template <template <typename> class Container, typename T,
          decltype(std::declval<Container<T>>().insert(std::declval<T>())) * =
              nullptr>
void insert(Container<T> &container, T element) {
    container.insert(element);
}

// sequence container
template <template <typename> class Container, typename T,
          decltype(std::declval<Container<T>>().push_back(
              std::declval<T>())) * = nullptr>
void insert(Container<T> &container, T element) {
    container.push_back(element);
}

template <typename T>
using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

template <typename T>
using iter_value_t = typename base_t<T>::iterator::value_type;

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
    template <typename T, std::enable_if_t<std::is_fundamental_v<              \
                              std::remove_reference_t<T>>> * = nullptr>        \
    static std::remove_reference_t<T> deserialize(const std::string &str) {    \
        std::remove_reference_t<T> t;                                          \
        std::istringstream(str) >> t;                                          \
        return t;                                                              \
    }                                                                          \
                                                                               \
    template <typename T, std::enable_if_t<                                    \
                              std::is_pointer_v<std::remove_reference_t<T>> && \
                              !std::is_abstract_v<std::remove_pointer_t<       \
                                  std::remove_reference_t<T>>>> * = nullptr>   \
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
    template <typename T, typename base_t<T>::iterator * = nullptr,            \
              std::enable_if_t<!std::is_same_v<base_t<T>, std::string>> * =    \
                  nullptr>                                                     \
    static base_t<T> deserialize(const std::string &str) {                     \
        base_t<T> result;                                                      \
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
    template <typename T,                                                      \
              std::enable_if_t<std::is_same_v<std::remove_reference_t<T>,      \
                                              std::string>> * = nullptr>       \
    static std::string deserialize(const std::string &str) {                   \
        std::string t = str.substr(1, str.size() - 2);                         \
        return t;                                                              \
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
    template <template <typename> class Container, typename T,                 \
              typename Container<T>::iterator * = nullptr>                     \
    static std::string serialize(const Container<T> &elts) {                   \
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
    }                                                                          \
                                                                               \
    static inline std::string serialize(std::string &elt) {                    \
        std::ostringstream oss;                                                \
        oss << "\"" << elt << "\"";                                            \
        return oss.str();                                                      \
    }

/******************************************************************************/
/*                          default convertor class                           */
/******************************************************************************/

struct Convertor {
    Convertor() = delete;
    virtual ~Convertor() {}
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

#define class_name(Type) typeid(Type).name()

#endif
