#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include "metafunctions.hpp"
#include <type_traits>

namespace serializer::concepts {

template <typename T>
concept Serializable = requires(T obj) { obj.serialize(); };

template <typename T>
concept SerializablePtr = requires(T obj) { obj->serialize(); };

template <typename T>
concept Deserializable = requires(T obj) { obj.deserialize(""); };

template <typename T>
concept DeserializablePtr = requires(T obj) { obj->deserialize(""); };

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

template <typename T>
concept TupleLike = requires(T obj) {
    std::get<0>(obj);
    typename std::tuple_element_t<0, T>;
    std::tuple_size_v<T>;
};

template <typename T>
concept Array = mtf::is_std_array_v<T>;

/* unsupported types */

template <typename T>
concept AutoSerializationSupported =
    !SmartPtr<T> && !Pointer<T> && !Fundamental<T> &&
    !Enum<T> && !String<T> && !Iterable<T> && !TupleLike<T>;

template <typename T>
concept AutoDeserializationSupported =
!SmartPtr<T> && !ConcretePtr<T> && !Fundamental<T> &&
!Enum<T> && !String<T> && !Iterable<T> && !TupleLike<T>;

template <typename T>
concept NonSerializable = !Serializable<T> && AutoSerializationSupported<T>;

template <typename T>
concept NonDeserializable = !Deserializable<T> && AutoDeserializationSupported<T>;

/* for insert helper function */

template <typename Container, typename T>
concept Insertable = requires (Container obj) {
    obj.insert(std::declval<T>());
};

template <typename Container, typename T>
concept PushBackable = requires (Container obj) {
    obj.push_back(std::declval<T>());
};

}; // namespace serializer::concepts

#endif
