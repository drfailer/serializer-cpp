#ifndef PARSER_HPP
#define PARSER_HPP
#include <stack>
#include <string>

/*
 * findEndValueIndex
 *
 * find the index of the end of the value starting at valueStart.
 */
inline std::size_t findEndValueIndex(const std::string &str,
                                     std::size_t valueStart) {
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

/*
 * nextId
 *
 * find the index of the nex identifier in str.
 */
inline std::size_t nextId(const std::string &str) {
    return str.find(",") + 2; // ..., id
}

/*
 * getClassName
 *
 * return the name of the class that is deserilized (stored in str).
 */
inline std::string getClassName(const std::string &str) {
    constexpr std::size_t classNameIdStartIndex = 2; // { __CLASS_NAME__:
    constexpr std::size_t classNameIdLength = 14;    // __CLASS_NAME__
    constexpr std::size_t valueBegin =
        classNameIdStartIndex + classNameIdLength + 2; // : value
    std::size_t valueEnd = findEndValueIndex(str, valueBegin);
    return str.substr(valueBegin, valueEnd - valueBegin);
}

#endif
