#ifndef SERIALIZER_CONCEPTS_H
#define SERIALIZER_CONCEPTS_H
#include "type_check.hpp"

/// @brief namespace serializer concepts
namespace serializer::concepts {

/// @brief Objects that have a serialize member function.
template <typename T>
concept Serializable =
    requires(T obj, tools::vec<uint8_t> &mem) { obj.serialize(mem, 0); };

/// @brief Objects that have a deserialize member function.
template <typename T>
concept Deserializable =
    requires(T obj, tools::vec<uint8_t> &mem) { obj.deserialize(mem, 0); };

/// @brief Smart pointers.
template <typename T>
concept SmartPtr = mtf::is_smart_ptr_v<T>;

/// @brief Smart pointers.
template <typename T>
concept ConcreteSmartPtr =
    SmartPtr<T> && !std::is_abstract_v<mtf::element_type_t<T>>;

/// @brief Concreate pointer.
template <typename T>
concept ConcretePtr = mtf::is_concrete_ptr_v<T>;

/// @brief Pointers
template <typename T>
concept Pointer = std::is_pointer_v<mtf::base_t<T>>;

/// @brief Static arrays
template <typename T>
concept StaticArray = std::is_array_v<mtf::base_t<T>>;

/// @brief Match std::array.
template <typename T>
concept Array = mtf::is_std_array_v<T>;

/// @brief Trivial types that can be cast directly
template <typename T>
concept Trivial = !std::is_pointer_v<mtf::base_t<T>> && !Array<T> &&
                  !StaticArray<T> && !Serializable<T> && !Deserializable<T> &&
                  std::is_copy_assignable_v<mtf::base_t<T>> &&
                  std::is_trivially_copyable_v<mtf::base_t<T>>;

/// @brief Enum types.
template <typename T>
concept Enum = std::is_enum_v<mtf::base_t<T>>;

/// @brief String types.
template <typename T>
concept String = mtf::is_string_v<T>;

/// @brief Iterable types.
template <typename T>
concept Iterable = requires { typename mtf::base_t<T>::iterator; };

/// @brief The object has a clear member function.
template <typename T>
concept Clearable = requires(mtf::base_t<T> obj) { obj.clear(); };

/// @brief The object has a resize member function.
template <typename T>
concept Resizeable = requires(mtf::base_t<T> obj) { obj.resize(1); };

/// @brief Iterable types that are not strings (string are handled differently
///        for optimization and readability).
template <typename T>
concept Container = !String<T> && Iterable<T>;

/// @brief Match tuples (exists in C++23).
template <typename T>
concept TupleLike = requires(mtf::base_t<T> obj) {
    std::get<0>(obj);
    typename std::tuple_element_t<0, mtf::base_t<T>>;
    std::tuple_size_v<mtf::base_t<T>>;
} && !Array<T>;

/// @brief Match types on which we can use std::forward
template <typename T>
concept Forwardable = requires(T &&obj) { std::forward<T>(obj); };

/* unsupported types */

/// @brief Used to detect the types for which we do not have an automatic
///        serialization function.
template <typename T>
concept AutoSerializationSupported =
    SmartPtr<T> || Pointer<T> || Trivial<T> || Enum<T> || String<T> ||
    Iterable<T> || TupleLike<T> || StaticArray<T>;

/// @brief Used to detect the types for which we do not have an automatic
///        deserialization function.
template <typename T>
concept AutoDeserializationSupported =
    ConcreteSmartPtr<T> || ConcretePtr<T> || Trivial<T> || Enum<T> ||
    String<T> || Iterable<T> || TupleLike<T> || StaticArray<T>;

/// @brief Detect if a type is serializable.
template <typename T>
concept NonSerializable = !Serializable<T> && !AutoSerializationSupported<T>;

/// @brief Detect if a type is deserializable.
template <typename T>
concept NonDeserializable =
    !Deserializable<T> && !AutoDeserializationSupported<T>;

/* for insert helper function */

/// @brief Used to detect whether a members_ uses insert or not.
template <typename Container, typename T>
concept Insertable = requires(Container obj) { obj.insert(std::declval<T>()); };

/// @brief Used to detect whether a members_ uses add or not.
template <typename Container, typename T>
concept PushBackable =
    requires(Container obj) { obj.push_back(std::declval<T>()); };

}; // namespace serializer::concepts

/// @brief Serializer function type. It should be a concepts but compilers
///        cannot deduce the type of the lambda.
///        TODO: this should not be defined here
#define SerializerFunction(func, conv)                                         \
    requires { func(Context<Phases::Serialization, decltype(conv)>(conv)); }

#endif
