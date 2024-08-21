#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include "../tools/vec.hpp"
#include "metafunctions.hpp"
#include <type_traits>

namespace serializer::tools::concepts {

/// @brief Objects that have a serialize member function.
/// TODO: change str
template <typename T>
concept Serializable =
    requires(mtf::base_t<T> obj, vec<char> &mem) { obj.serialize(mem); };

/// @brief Objects that have a deserialize member function.
/// TODO: change str
template <typename T>
concept Deserializable =
    requires(mtf::base_t<T> obj, vec<char> &mem) { obj.deserialize(mem); };

/// @brief Smart pointers.
template <typename T>
concept SmartPtr = mtf::is_smart_ptr_v<mtf::base_t<T>>;

/// @brief Smart pointers.
template <typename T>
concept ConcreteSmartPtr =
    SmartPtr<T> && !std::is_abstract_v<typename mtf::base_t<T>::element_type>;

/// @brief Concreate pointer.
template <typename T>
concept ConcretePtr = mtf::is_concrete_ptr_v<mtf::base_t<T>>;

/// @brief Pointers
template <typename T>
concept Pointer = std::is_pointer_v<mtf::base_t<T>>;

/// @brief Static arrays
template <typename T>
concept StaticArray = std::is_array_v<mtf::base_t<T>>;

/// @brief Fundamental types
template <typename T>
concept Fundamental = std::is_fundamental_v<mtf::base_t<T>>;

/// @brief Enum types.
template <typename T>
concept Enum = std::is_enum_v<mtf::base_t<T>>;

/// @brief String types.
template <typename T>
concept String = mtf::is_string_v<mtf::base_t<T>>;

/// @brief Iterable types.
template <typename T>
concept Iterable = requires { typename mtf::base_t<T>::iterator; };

/// @brief The object has a clear memeber function.
template <typename T>
concept Clearable = requires(mtf::base_t<T> obj) { obj.clear(); };

/// @brief Iterable types that are not strings (string are handled differently
///        for optimization and readability).
template <typename T>
concept NonStringIterable = !String<T> && Iterable<T>;

/// @brief Match tuples (exists in C++23).
template <typename T>
concept TupleLike = requires(mtf::base_t<T> obj) {
    std::get<0>(obj);
    typename std::tuple_element_t<0, mtf::base_t<T>>;
    std::tuple_size_v<mtf::base_t<T>>;
};

/// @brief Remove const on all types
/// TODO: move this elsewhere
template <typename T> struct remove_const {
    using type = std::remove_const_t<T>;
};

template <TupleLike T> struct remove_const<T> {
    using type = mtf::remove_const_tuple_t<T>;
};

template <typename T> using remove_const_t = typename remove_const<T>::type;

/// @brief Match std::array.
template <typename T>
concept Array = mtf::is_std_array_v<mtf::base_t<T>>;

/// @brief Match types on which we can use std::forward
template <typename T>
concept Forwardable = requires(T &&obj) { std::forward<T>(obj); };

/* unsupported types */

/// @brief Used to detect the types for which we do not have an automatic
///        serialization function.
template <typename T>
concept AutoSerializationSupported =
    SmartPtr<T> || Pointer<T> || Fundamental<T> || Enum<T> || String<T> ||
    Iterable<T> || TupleLike<T> || StaticArray<T>;

/// @brief Used to detect the types for which we do not have an automatic
///        deserialization function.
template <typename T>
concept AutoDeserializationSupported =
    ConcreteSmartPtr<T> || ConcretePtr<T> || Fundamental<T> || Enum<T> ||
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

}; // namespace serializer::tools::concepts

/// @brief Serializer function type. It should be a concepts but compilers
///        cannot deduce the type of the lambda.
///        TODO: this should not be defined here
#define SerializerFunction(func, conv)                                         \
    requires { func(Context<Phases::Serialization, decltype(conv)>(conv)); }

#endif
