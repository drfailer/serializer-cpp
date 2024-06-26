#ifndef PARSER_HPP
#define PARSER_HPP
#include <map>
#include <stack>
#include <string>
#include <utility>

namespace serializer::parser {

/******************************************************************************/
/*                         general parsing functions                          */
/******************************************************************************/

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
        if (str[idx] == '\\') {
            idx += 2;
        }
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
 * parseOneLvl
 *
 * Parse on level of a serialized object string. Parsing on level means that we
 * only treat ids and values in the current brackets without treating nested
 * elements ("{ level }", "{ id: { nested element } }").
 */
inline std::map<std::string, std::string> parseOneLvl(const std::string &str) {
    std::map<std::string, std::string> result;
    std::size_t idx = 2; // { ...
    std::size_t end;

    while (idx < str.size() - 1) {
        /* withdraw the id */
        end = str.find(':', idx);
        // if we remove the spaces in the serialized string, this won't be a
        // problem anymore:
        if (end >= str.size())
            break; // security for the empty serilizer (there
                   // is an empty value at the end)
        std::string id = str.substr(idx, end - idx);

        /* withdraw the value */
        idx += id.size() + 2; // id: ...
        end = findEndValueIndex(str, idx);
        std::string value = str.substr(idx, end - idx);
        idx = end + 2;
        result.insert(std::make_pair(id, value));
    }
    return result;
}

/*
 * nextId
 *
 * find the index of the nex identifier in str.
 */
inline std::size_t nextId(const std::string &str) {
    return str.find(",") + 2; // ..., id
}

/******************************************************************************/
/*                                 get values                                 */
/******************************************************************************/

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

/******************************************************************************/
/*                                   tuples                                   */
/******************************************************************************/

inline std::pair<std::string, std::string> parseTuple(const std::string &str) {
    std::string value;
    std::string rest;
    std::size_t begin = 2;
    std::size_t end = findEndValueIndex(str, begin);

    value = str.substr(begin, end - begin);
    rest = str.substr(end);
    return std::make_pair(value, rest);
}

/******************************************************************************/
/*                               escape strings                               */
/******************************************************************************/

inline std::string escapeStr(const std::string &str) {
    std::string result = "";

    for (char c : str) {
        if (c == '"' || c == '\\') {
            result.push_back('\\');
        }
        result.push_back(c);
    }
    return result;
}

inline std::string unescapeStr(const std::string &str) {
    std::string result = "";
    std::string::const_iterator it = str.begin();

    for (; it != str.end(); it++) {
        if (*it == '\\' && (*(it + 1) == '"' || *(it + 1) == '\\')) {
            it++;
            result.push_back(*it);
        } else {
            result.push_back(*it);
        }
    }
    return result;
}

} // namespace serializer::parser

#endif
