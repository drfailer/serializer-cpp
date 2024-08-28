#ifndef SERIALIZER_SUPER_H
#define SERIALIZER_SUPER_H
#include <cstddef>

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Wrapper class for serializing the mother class of polymorphic
///        objects.
/// @tparm SuperType Type of the mother class.
template <typename SuperType> struct Super {
    SuperType *obj;

    /// @brief Constructor from object.
    /// @param obj Pointer to `this` of the serialized class.
    constexpr Super(SuperType *obj) : obj(obj) {}

    /// @brief Call the serialize method of the super class.
    /// @param mem Buffer in which the serialized data is be stored.
    /// @param pos Start position in the buffer for serializing the data.
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {
        return obj->SuperType::serialize(mem, pos);
    }

    /// @brief Call the deserialize method of the super class.
    /// @param mem Buffer in which the serialized data is be stored.
    /// @param pos Start position in the buffer for deserializing the data.
    constexpr size_t deserialize(auto &mem, size_t pos = 0) {
        return obj->SuperType::deserialize(mem, pos);
    }
};

/// @brief Helper function for creating a Super from an object.
/// @param obj Pointer to the object (this).
template <typename T> inline constexpr auto super(auto *obj) {
    return Super<T>(static_cast<T *>(obj));
}

/// @brief Helper function for creating a Super from an const object.
/// @param obj Pointer to the object (this).
template <typename T> inline constexpr auto super(auto const *obj) {
    return Super<const T>(static_cast<T const *>(obj));
}

} // end namespace serializer::tools

#endif
