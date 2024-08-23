#ifndef SERIALIZER_CONTEXT_H
#define SERIALIZER_CONTEXT_H

namespace serializer {

/// @brief Enum used with the serializer functions to know the stage of the
///        serialization.
enum class Phases { Serialization, Deserialization };

/// @brief Reporesents the context given the the serializer functions.
/// @tparam Phase     Phase of the serialization
/// @tparam Convertor Convertor that holds the byte array and the serialization
///                   functions.
template <Phases Phase, typename Convertor>
struct Context {
  static constexpr Phases phase = Phase;
  Convertor convertor;
};

} // end namespace serializer

#endif
