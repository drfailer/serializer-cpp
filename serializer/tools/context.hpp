#ifndef SERIALIZER_CONTEXT_H
#define SERIALIZER_CONTEXT_H

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Enum used with the serializer functions to know the stage of the
///        serialization.
enum class Phases { Serialization, Deserialization };

/// @brief Reporesents the context given the the serializer functions.
/// @tparam Phase      Phase of the serialization
/// @tparam Serializer Serializer that holds the byte array and the
///                    serialization functions.
template <Phases Phase, typename Serializer> struct Context {
    static constexpr Phases phase = Phase; ///< phase
    Serializer serializer;                 ///< serializer
};

} // namespace serializer::tools

#endif
