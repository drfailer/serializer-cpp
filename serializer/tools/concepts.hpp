#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include "metafunctions.hpp"
#include <type_traits>

namespace serializer::concepts {

/// @brief Objects that have a serialize member function.
template <typename T>
concept Serializable = requires(T obj) { obj.serialize(); };

/// @brief Objects that have a deserialize member function.
template <typename T>
concept Deserializable = requires(T obj) { obj.deserialize(""); };

/// @brief Smart pointers.
template <typename T>
concept SmartPtr = mtf::is_smart_ptr_v<T>;

/// @brief Smart pointers.
template <typename T>
concept ConcreteSmartPtr =
    SmartPtr<T> && !std::is_abstract_v<typename T::element_type>;

/// @brief Concreate pointer.
template <typename T>
concept ConcretePtr = mtf::is_concrete_ptr_v<T>;

/// @brief Pointer
template <typename T>
concept Pointer = std::is_pointer_v<T>;

/// @brief Fundamental types
template <typename T>
concept Fundamental = std::is_fundamental_v<T>;

/// @brief Enum types.
template <typename T>
concept Enum = std::is_enum_v<T>;

/// @brief String types.
template <typename T>
concept String = mtf::is_string_v<T>;

/// @brief Iterable types.
template <typename T>
concept Iterable = requires { typename T::iterator; };

/// @brief Iterable types that are not strings (string are handled differently
///        for optimization and readability).
template <typename T>
concept NonStringIterable = !String<T> && Iterable<T>;

/// @brief Match tuples (exists in C++23).
template <typename T>
concept TupleLike = requires(T obj) {
    std::get<0>(obj);
    typename std::tuple_element_t<0, T>;
    std::tuple_size_v<T>;
};

/// @brief Match std::array.
template <typename T>
concept Array = mtf::is_std_array_v<T>;

/// @brief Match types on which we can use std::forward
template <typename T>
concept Forwardable = requires(T &&obj) {
    std::forward<T>(obj);
};

/* unsupported types */

/// @brief Used to detect the types for which we do not have an automatic
///        serialization function.
template <typename T>
concept AutoSerializationSupported =
    !SmartPtr<T> && !Pointer<T> && !Fundamental<T> && !Enum<T> && !String<T> &&
    !Iterable<T> && !TupleLike<T>;

/// @brief Used to detect the types for which we do not have an automatic
///        deserialization function.
template <typename T>
concept AutoDeserializationSupported =
    !ConcreteSmartPtr<T> && !ConcretePtr<T> && !Fundamental<T> && !Enum<T> &&
    !String<T> && !Iterable<T> && !TupleLike<T>;

/// @brief Detect if a type is serializable.
template <typename T>
concept NonSerializable = !Serializable<T> && AutoSerializationSupported<T>;

/// @brief Detect if a type is deserializable.
template <typename T>
concept NonDeserializable =
    !Deserializable<T> && AutoDeserializationSupported<T>;

/* for insert helper function */

/// @brief Used to detect whether a members_ uses insert or not.
template <typename Container, typename T>
concept Insertable = requires(Container obj) { obj.insert(std::declval<T>()); };

/// @brief Used to detect whether a members_ uses add or not.
template <typename Container, typename T>
concept PushBackable =
    requires(Container obj) { obj.push_back(std::declval<T>()); };

}; // namespace serializer::concepts

#endif
