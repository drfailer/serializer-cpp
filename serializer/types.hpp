#ifndef TYPES_HPP
#define TYPES_HPP

namespace serializer {

/// @brief Enum used in the serializer function to know the stage of the
///        serialization.
enum class Phases { Serialization, Deserialization };

template <Phases phase, typename Conv>
struct Context {
  Conv convertor;
};

} // end namespace serializer

#endif
