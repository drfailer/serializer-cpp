#ifndef HANDLERS_HPP
#define HANDLERS_HPP
#include "parser.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits>

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

// sorted container
template <template <typename> class Container, typename T,
          decltype(std::declval<Container<T>>().insert(T())) * = nullptr>
void insert(Container<T> &container, T element) {
    container.insert(element);
}

// sequence container
template <template <typename> class Container, typename T,
          decltype(std::declval<Container<T>>().push_back(T())) * = nullptr>
void insert(Container<T> &container, T element) {
    container.push_back(element);
}

/******************************************************************************/
/*                    serialize and deserialize functions                     */
/******************************************************************************/

/* This functions are used to switch between string and values. */

struct Convertor {
    Convertor() = default;
    virtual ~Convertor() {}

    /* deserialize ************************************************************/

    template <typename T,
              decltype(std::declval<T>().deserialize("")) * = nullptr>
    std::remove_reference_t<T> deserialize(const std::string &str) {
        std::remove_reference_t<T> t;
        t.deserialize(str);
        return t;
    }

    template <typename T, std::enable_if_t<std::is_fundamental_v<
                              std::remove_reference_t<T>>> * = nullptr>
    std::remove_reference_t<T> deserialize(const std::string &str) {
        std::remove_reference_t<T> t;
        std::istringstream(str) >> t;
        return t;
    }

    template <typename T,
              std::enable_if_t<std::is_pointer_v<std::remove_reference_t<T>>>
                  * = nullptr>
    std::remove_reference_t<T> deserialize(const std::string &str) {
        if (str == "nullptr") {
            return nullptr;
        }
        using Type = typename std::remove_pointer_t<std::remove_reference_t<T>>;
        Type *t = new Type();

        if constexpr (std::is_fundamental_v<Type>) {
            *t = deserialize<Type>(str);
        } else {
            t->deserialize(str);
        }
        return t;
    }

    template <typename T>
    using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

    template <typename T>
    using iter_value_t = typename base_t<T>::iterator::value_type;

    template <
        typename T, typename base_t<T>::iterator * = nullptr,
        std::enable_if_t<!std::is_same_v<base_t<T>, std::string>> * = nullptr,
        std::enable_if_t<
            !std::is_pointer_v<iter_value_t<T>> ||
            std::is_fundamental_v<std::remove_pointer_t<iter_value_t<T>>>> * =
            nullptr>
    base_t<T> deserialize(const std::string &str) {
        base_t<T> result;
        std::size_t valueStart = 2;
        std::size_t valueEnd;

        while (valueStart < str.size()) {
            valueEnd = findEndValueIndex(str, valueStart);
            insert(result, deserialize<iter_value_t<T>>(
                               str.substr(valueStart, valueEnd - valueStart)));
            valueStart = valueEnd + 2; // value1, value2
        }

        return result;
    }

    template <typename T,
              std::enable_if_t<std::is_same_v<std::remove_reference_t<T>,
                                              std::string>> * = nullptr>
    std::string deserialize(const std::string &str) {
        std::string t = str.substr(1, str.size() - 2);
        return t;
    }

    /* serialize **************************************************************/

    template <typename T, decltype(std::declval<T>().serialize()) * = nullptr>
    std::string serialize(T &elt) {
        return elt.serialize();
    }

    template <typename T,
              std::enable_if_t<std::is_fundamental_v<T>> * = nullptr>
    std::string serialize(T &elt) {
        std::ostringstream oss;
        oss << elt;
        return oss.str();
    }

    template <typename T, std::enable_if_t<std::is_pointer_v<T>> * = nullptr>
    std::string serialize(T elt) {
        if (elt != nullptr) {
            return serialize<std::remove_pointer_t<T>>(*elt);
        } else {
            return "nullptr";
        }
    }

    template <template <typename> class Container, typename T,
              typename Container<T>::iterator * = nullptr,
              std::enable_if_t<!std::is_pointer_v<T> ||
                               std::is_fundamental_v<std::remove_pointer_t<T>>>
                  * = nullptr>
    std::string serialize(const Container<T> &elts) {
        std::ostringstream oss;
        auto it = elts.begin();

        if (it != elts.end()) {
            oss << "[ ";
            for (; it != elts.end() - 1; it++) {
                oss << serialize(*it) << ", ";
            }
            oss << serialize(*it) << " ]";
        }
        return oss.str();
    }

    inline std::string serialize(std::string &elt) {
        std::ostringstream oss;
        oss << "\"" << elt << "\"";
        return oss.str();
    }
};

#endif
