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

inline std::string getValue(const std::string &str, const std::string &id) {
    std::size_t idStartIdx = str.find(id + ":");
    std::size_t idLength = id.size();
    std::size_t valueBegin = idStartIdx + idLength + 2;
    std::size_t valueEnd = findEndValueIndex(str, valueBegin);
    return str.substr(valueBegin, valueEnd - valueBegin);
}

inline std::string getSuperValue(const std::string &str) {
    return getValue(str, "__SUPER__");
}

inline std::string getThisValue(const std::string &str) {
    return getValue(str, "__THIS__");
}

inline std::string getThisClassName(const std::string &str) {
    return getValue(getThisValue(str), "__CLASS_NAME__");
}

inline std::string getClassName(const std::string &str) {
    return getValue(str, "__CLASS_NAME__");
}

#endif
