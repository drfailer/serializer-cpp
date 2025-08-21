#ifndef SERIALIZER_SERIALIZER_META_H
#define SERIALIZER_SERIALIZER_META_H
#include "concepts.hpp"
#include "type_check.hpp"
#include "type_transform.hpp"

/// @file This file contains concepts and metafunctions used only in the
///       serializer (avoid recursive includes)

/// @brief namespace serializer
namespace serializer {

/// @brief namespace concepts
namespace concepts {

/// @brief Container types that are contiguous and stored a trivial type
template <typename T, typename MemT>
concept ContiguousTrivial =
    std::contiguous_iterator<typename mtf::clean_t<T>::iterator> &&
    concepts::Trivial<mtf::remove_const_t<mtf::iter_value_t<mtf::clean_t<T>>>> &&
    !concepts::Serializable<
        mtf::remove_const_t<mtf::iter_value_t<mtf::clean_t<T>>>, MemT>;

/// @brief Trivialy serializable static arrays
template <typename T, typename MemT>
concept TrivialySerializableStaticArray =
    !std::is_array_v<std::remove_extent_t<mtf::clean_t<T>>> &&
    concepts::Trivial<std::remove_extent_t<mtf::clean_t<T>>> &&
    !concepts::Serializable<std::remove_extent_t<mtf::clean_t<T>>, MemT>;

/// @brief Trivialy deserializable static arrays
template <typename T, typename MemT>
concept TrivialyDeserializableStaticArray =
    !std::is_array_v<std::remove_extent_t<mtf::clean_t<T>>> &&
    concepts::Trivial<std::remove_extent_t<mtf::clean_t<T>>> &&
    !concepts::Deserializable<std::remove_extent_t<mtf::clean_t<T>>, MemT>;

} // end namespace concepts

} // end namespace serializer

#endif
