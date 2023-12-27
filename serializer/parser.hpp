#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>

std::size_t findEndValueIndex(const std::string& str, std::size_t valueStart);
std::size_t nextId(const std::string& str);
std::string getClassName(const std::string& str);

#endif
