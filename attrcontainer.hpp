#ifndef ATTRCONTAINER_HPP
#define ATTRCONTAINER_HPP
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>

/******************************************************************************/
/*                    serialize and deserialize functions                     */
/******************************************************************************/

namespace func {

template<typename T, decltype(std::declval<T>().delserialize())* = nullptr>
std::remove_reference_t<T> deserialize(std::string str) {
    std::remove_reference_t<T> t;
    t.delserialize(str);
    return t;
}

template<typename T, std::enable_if_t<std::is_fundamental_v<std::remove_reference_t<T>>>* = nullptr>
std::remove_reference_t<T> deserialize(std::string str) {
    std::remove_reference_t<T> t;
    std::istringstream(str) >> t;
    return t;
}

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

} // end namespace func

/******************************************************************************/
/*                              helper functions                              */
/******************************************************************************/

inline std::size_t findValueEnd(std::string str, std::size_t valueStart) {
    std::size_t idx = valueStart;
    std::stack<char> pairs;

    while (idx < str.size()) {
        if (idx == ' ' && pairs.empty()) {
            return idx;
        } else if (str[idx] == '{') {
            pairs.push('}');
        } else if (str[idx] == '[') {
            pairs.push(']');
        } else if (!pairs.empty() && str[idx] == pairs.top()) {
            pairs.pop();
        }
        idx++;
    }
    return idx;
}

inline std::size_t nextId(const std::string& str) {
    std::size_t idx = str.find(",") + 1;
    while (idx < str.size() && str[idx] == ' ') {
        idx++;
    }
    return idx;
}

/******************************************************************************/
/*                            attribute container                             */
/******************************************************************************/

template<typename ...Types>
struct AttrContainer;

template<typename H, typename ...Types>
struct AttrContainer<H, Types...> {
    H& reference;
    std::string name;
    AttrContainer<Types...> next;

    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << func::serialize(reference);
        oss << ", " << next.serialize();
        return oss.str();
    }

    void deserialize(const std::string& str) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findValueEnd(str, idxValue);
        reference = func::deserialize<decltype(reference)>(str.substr(idxValue, idxEnd));
        next.deserialize(str);
    }

    AttrContainer(H& head, Types&... types, const std::string& idsStr):
        reference(head),
        name(idsStr.substr(0, idsStr.find(","))),
        next(types..., idsStr.substr(nextId(idsStr)))
    { }
};

template<typename H>
struct AttrContainer<H> {
    H& reference;
    std::string name;

    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << func::serialize(reference);
        return oss.str();
    }

    void deserialize(const std::string& str) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findValueEnd(str, idxValue);
        reference = func::deserialize<decltype(reference)>(str.substr(idxValue, idxEnd));
    }

    AttrContainer(H& head, const std::string& idsStr):
        reference(head), name(idsStr) { }
};

template<>
struct AttrContainer<> {};

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

template<const char* chars>
struct getAttr {

};

#endif
