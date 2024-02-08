#ifndef PARSER_HPP
#define PARSER_HPP
#include <stack>
#include <string>
#include <utility>

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
 * Return the value of the with the key `id` in the serialized string.
 */
inline std::string getValue(const std::string &str, const std::string &id,
        bool strIsDefault = false) {
    std::string_view sv = str;
    std::size_t idLength = id.size();
    std::size_t valueBegin, valueEnd;
    std::size_t idStartIdx = 2; // { __firstid__: ... }

    // find the beginning of the string
    while (sv.substr(idStartIdx, idLength) != id) {
        valueBegin = sv.find(":", idStartIdx) + 2;
        idStartIdx = findEndValueIndex(str, valueBegin) + 2;

        if (idStartIdx >= str.size()) {
            return strIsDefault ? str : "";
        }
    }

    // get the value
    valueBegin = idStartIdx + idLength + 2;
    valueEnd = findEndValueIndex(str, valueBegin);
    return str.substr(valueBegin, valueEnd - valueBegin);
}

inline std::string getSuperValue(const std::string &str) {
    return getValue(str, "__SUPER__");
}

inline std::string getThisValue(const std::string &str) {
    return getValue(str, "__THIS__", true);
}

inline std::string getThisClassName(const std::string &str) {
    return getValue(getThisValue(str), "__CLASS_NAME__");
}

inline std::string getClassName(const std::string &str) {
    return getValue(str, "__CLASS_NAME__");
}

inline std::pair<std::string, std::string> parsePair(const std::string& str) {
    std::string elt1;
    std::string elt2;
    std::size_t begin = 2;
    std::size_t end = findEndValueIndex(str, begin);

    elt1 = str.substr(begin, end - begin);
    begin = end + 2;
    end = str.size() - 2;
    elt2 = str.substr(begin, end - begin);
    return std::make_pair(elt1, elt2);
}

#endif
