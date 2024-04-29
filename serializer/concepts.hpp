#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include <type_traits>
#include "metafunctions.hpp"

namespace serializer::concepts {

template <typename T>
concept Serializable = requires (T obj) { obj.serialize(); };

template <typename T>
concept Deserializable = requires (T obj) { obj.deserialize(""); };

template <typename T>
concept SmartPtr = mtf::is_smart_ptr_v<T>;

template <typename T>
concept ConcretePtr = mtf::is_concrete_ptr_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

template <typename T>
concept Fundamental = std::is_fundamental_v<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept String = mtf::is_string_v<T>;

template <typename T>
concept Iterable = requires { typename T::iterator; };

template <typename T>
concept NonStringIterable = !String<T> && Iterable<T>;

}; // namespace serializer::concepts

#endif
