#ifndef SERIALIZER_SERIALIZER_SERIALIZE_HPP
#define SERIALIZER_SERIALIZER_SERIALIZE_HPP

namespace serializer {

/// @brief Used to create a serialize behavior for a specific type. This class
///        is used when we want to create a custom serializer that add support
///        for external types.
/// @param T Type for which we want to add the serialize behavior for.
template <typename T> struct Serialize {
    constexpr virtual void serialize(T const &) = 0;
    constexpr virtual void deserialize(T &) = 0;
};

}

#endif // SERIALIZER_SERIALIZER_SERIALIZE_HPP
