#ifndef TYPES_HPP
#define TYPES_HPP
#include <functional>
#include <string_view>

namespace serializer {

/// @brief Enum used in the serializer function to know the stage of the
///        serialization.
enum class Phases { Serialization, Deserialization };

/// @brief Serializer function type (functions that can be given the the
///        serializer and which are executed during the serialization /
///        deserialization).
using function_t = std::function<void(Phases, std::string_view const &)>;

} // end namespace serializer

#endif
