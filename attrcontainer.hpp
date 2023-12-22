#ifndef ATTRCONTAINER_HPP
#define ATTRCONTAINER_HPP
#include <algorithm>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <iostream>

/******************************************************************************/
/*                    serialize and deserialize functions                     */
/******************************************************************************/

/* This functions are used to switch between string and values. */

namespace func {

/* deserialize ****************************************************************/

template<typename T, decltype(std::declval<T>().deserialize(""))* = nullptr>
std::remove_reference_t<T> deserialize(const std::string& str) {
    std::remove_reference_t<T> t;
    t.deserialize(str);
    return t;
}

template<typename T, std::enable_if_t<std::is_fundamental_v<std::remove_reference_t<T>>>* = nullptr>
std::remove_reference_t<T> deserialize(const std::string& str) {
    std::remove_reference_t<T> t;
    std::istringstream(str) >> t;
    return t;
}

template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<T>, std::string>>* = nullptr>
std::string deserialize(const std::string& str) {
    std::string t = str.substr(1, str.size() - 2);
    return t;
}

template<typename T, std::enable_if_t<std::is_pointer_v<std::remove_reference_t<T>>>* = nullptr>
std::remove_reference_t<T> deserialize(const std::string& str) {
    if (str == "nullptr") {
        return nullptr;
    }
    using Type = typename std::remove_pointer_t<std::remove_reference_t<T>>;
    Type *t = new Type();

    if constexpr(std::is_fundamental_v<Type>) {
        *t = deserialize<Type>(str);
    } else {
        t->deserialize(str);
    }
    return t;
}

/* serialize ******************************************************************/

template<typename T, decltype(std::declval<T>().serialize())* = nullptr>
std::string serialize(T& elt) {
    return elt.serialize();
}

template<typename T, std::enable_if_t<std::is_fundamental_v<T>>* = nullptr>
std::string serialize(T& elt) {
    std::ostringstream oss;
    oss << elt;
    return oss.str();
}

template<typename T, std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
std::string serialize(T elt) {
    if (elt != nullptr) {
        return serialize<std::remove_pointer_t<T>>(*elt);
    } else {
        return "nullptr";
    }
}

inline std::string serialize(std::string& elt) {
    std::ostringstream oss;
    oss << "\"" << elt << "\"";
    return oss.str();
}

} // end namespace func

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

inline std::size_t findEndValueIndex(std::string str, std::size_t valueStart) {
    std::size_t idx = valueStart;
    std::stack<char> pairs;

    while (idx < str.size()) {
        if ((str[idx] == ',' || str[idx] == ' ') && pairs.empty()) {
            return idx;
        } else if (str[idx] == '{') {
            pairs.push('}');
        } else if (str[idx] == '[') {
            pairs.push(']');
        } else if (str[idx] == '"') {
            if (!pairs.empty() && pairs.top() == '"') {
                pairs.pop();
            } else {
                pairs.push('"');
            }
        } else if (!pairs.empty() && str[idx] == pairs.top()) {
            pairs.pop();
        }
        idx++;
    }
    return idx;
}

inline std::size_t nextId(const std::string& str) {
    return str.find(",") + 2; // ..., id
}

/******************************************************************************/
/*                            attribute container                             */
/******************************************************************************/

/*
 * Data structure that acts like a map with multiple types. It is used to store
 * the identifiers and the references on the attributes of the serialized class.
 */
template<typename ...Types>
struct AttrContainer { };

/*
 * Specialisation for handeling more than one type.
 */
template<typename H, typename ...Types>
struct AttrContainer<H, Types...> {
    /* attributes *************************************************************/
    H& reference;
    std::string name;
    AttrContainer<Types...> next;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << func::serialize(reference);
        oss << ", " << next.serialize();
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string& str) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr(std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = func::deserialize<H&>(str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = func::deserialize<H&>(str.substr(idxValue, idxEnd - idxValue));
        }
        next.deserialize(str);
    }

    /* constructor ************************************************************/
    AttrContainer(H& head, Types&... types, const std::string& idsStr):
        reference(head),
        name(idsStr.substr(0, idsStr.find(","))),
        next(types..., idsStr.substr(nextId(idsStr)))
    { }
};

/*
 * Specialisation with one type. This specialisation doesn't have the next
 * attribute.
 */
template<typename H>
struct AttrContainer<H> {
    /* attributes *************************************************************/
    H& reference;
    std::string name;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << func::serialize(reference);
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string& str) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr(std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = func::deserialize<H&>(str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = func::deserialize<H&>(str.substr(idxValue, idxEnd - idxValue));
        }

        reference = func::deserialize<decltype(reference)>(str.substr(idxValue, idxEnd - idxValue));
    }

    /* constructor ************************************************************/
    AttrContainer(H& head, const std::string& idsStr):
        reference(head), name(idsStr) { }
};

#endif
