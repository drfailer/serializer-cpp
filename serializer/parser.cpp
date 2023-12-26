#include <stack>
#include "parser.hpp"

std::size_t findEndValueIndex(std::string str, std::size_t valueStart) {
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

std::size_t nextId(const std::string& str) {
    return str.find(",") + 2; // ..., id
}
