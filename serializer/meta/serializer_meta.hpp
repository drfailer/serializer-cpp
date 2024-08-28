#ifndef SERIALIZER_SERIALIZER_META_H
#define SERIALIZER_SERIALIZER_META_H
#include "../tools/dynamic_array.hpp"
#include "concepts.hpp"
#include "type_check.hpp"
#include "type_transform.hpp"

/// @file This file contains concepts and metafunctions used only in the
///       serializer (avoid recursive includes)

/// @brief namespace serializer
namespace serializer {

/// @brief namespace meta-functions
namespace mtf {

/// @brief True if T is a DynamicArray, false otherwise
template <typename T> struct is_dynamic_array : std::false_type {};

template <typename T, typename... Sizes>
struct is_dynamic_array<tools::DynamicArray<T, Sizes...>> : std::true_type {};

/// @brief True if T is a DynamicArray, false otherwise
template <typename T>
constexpr bool is_dynamic_array_v = is_dynamic_array<base_t<T>>::value;

} // end namespace mtf

/// @brief namespace concepts
namespace concepts {

/// @brief Types that are not serialized automatically (custom serializer /
///        error).
template <typename T, typename MemT, typename... AdditionalTypes>
concept NonAutomaticSerialize =
    mtf::contains_v<T, AdditionalTypes...> ||
    (concepts::NonSerializable<T, MemT> && !mtf::is_dynamic_array_v<T>);

/// @brief Types that are not deserialized automatically (custom serializer /
///        error).
template <typename T, typename MemT, typename... AdditionalTypes>
concept NonAutomaticDeserialize =
    mtf::contains_v<T, AdditionalTypes...> ||
    (concepts::NonDeserializable<T, MemT> && !mtf::is_dynamic_array_v<T>);

/// @brief Types that use a serialize method
template <typename T, typename MemT, typename... AdditionalTypes>
concept UseSerialize =
    concepts::Serializable<T, MemT> && !mtf::contains_v<T, AdditionalTypes...>;

/// @brief Types that use a deserialize method
template <typename T, typename MemT, typename... AdditionalTypes>
concept UseDeserialize = serializer::concepts::Deserializable<T, MemT> &&
                         !mtf::contains_v<T, AdditionalTypes...>;

/// @brief Container types that are contiguous and stored a trivial type
template <typename T, typename MemT>
concept ContiguousTrivial =
    std::contiguous_iterator<typename mtf::base_t<T>::iterator> &&
    concepts::Trivial<mtf::remove_const_t<mtf::iter_value_t<mtf::base_t<T>>>> &&
    !concepts::Serializable<
        mtf::remove_const_t<mtf::iter_value_t<mtf::base_t<T>>>, MemT>;

/// @brief Trivialy serializable static arrays
template <typename T, typename MemT>
concept TrivialySerializableStaticArray =
    !std::is_array_v<std::remove_extent_t<mtf::base_t<T>>> &&
    concepts::Trivial<std::remove_extent_t<mtf::base_t<T>>> &&
    !concepts::Serializable<std::remove_extent_t<mtf::base_t<T>>, MemT>;

/// @brief Trivialy deserializable static arrays
template <typename T, typename MemT>
concept TrivialyDeserializableStaticArray =
    !std::is_array_v<std::remove_extent_t<mtf::base_t<T>>> &&
    concepts::Trivial<std::remove_extent_t<mtf::base_t<T>>> &&
    !concepts::Deserializable<std::remove_extent_t<mtf::base_t<T>>, MemT>;

} // end namespace concepts

} // end namespace serializer

#endif
