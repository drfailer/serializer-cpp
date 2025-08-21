#ifndef SERIALIZER_CONCEPTS_H
#define SERIALIZER_CONCEPTS_H
#include "type_check.hpp"

/// @brief namespace serializer concepts
namespace serializer::concepts {

/// @brief Objects that have a serialize member function.
template <typename T, typename MemT>
concept Serializable =
    requires(mtf::clean_t<T> obj, MemT &mem) { obj.serialize(mem, 0); };

/// @brief Objects that have a deserialize member function.
template <typename T, typename MemT>
concept Deserializable =
    requires(mtf::clean_t<T> obj, MemT &mem) { obj.deserialize(mem, 0); };

/// @brief Smart pointers.
template <typename T>
concept SmartPtr = mtf::is_smart_ptr_v<T>;

/// @brief Smart pointers.
template <typename T>
concept ConcreteSmartPtr =
    SmartPtr<T> && !std::is_abstract_v<mtf::element_type_t<T>>;

/// @brief Concreate pointer.
template <typename T>
concept ConcreteStdPtr = mtf::is_concrete_ptr_v<T>;

/// @brief All concreate pointers.
template <typename T>
concept ConcretePtr = ConcreteStdPtr<T> || ConcreteSmartPtr<T>;

/// @brief Pointers
template <typename T>
concept StdPointer = std::is_pointer_v<mtf::clean_t<T>>;

/// @brief All pointers
template <typename T>
concept Pointer = StdPointer<T> || SmartPtr<T>;

/// @brief Static arrays
template <typename T>
concept StaticArray = std::is_array_v<mtf::clean_t<T>>;

/// @brief Match std::array.
template <typename T>
concept Array = mtf::is_std_array_v<T>;

/// @brief Enum types.
template <typename T>
concept Enum = std::is_enum_v<mtf::clean_t<T>>;

/// @brief Trivial types that can be cast directly
template <typename T>
concept Trivial =
    !std::is_pointer_v<mtf::clean_t<T>> && !Array<T> && !StaticArray<T> &&
    !Enum<T> && std::is_copy_assignable_v<mtf::clean_t<T>> &&
    std::is_trivially_copyable_v<mtf::clean_t<T>>;

/// @brief String types.
template <typename T>
concept String = mtf::is_string_v<T>;

/// @brief Iterable types.
template <typename T>
concept Iterable = requires { typename mtf::clean_t<T>::iterator; };

/// @brief The object has a clear member function.
template <typename T>
concept Clearable = requires(mtf::clean_t<T> obj) { obj.clear(); };

/// @brief The object has a resize member function.
template <typename T>
concept Resizeable = requires(mtf::clean_t<T> obj) { obj.resize(1); };

/// @brief Contiguous resizeable containers
template <typename T>
concept ContiguousResizeable =
    std::contiguous_iterator<typename mtf::clean_t<T>::iterator> &&
    Resizeable<T>;

/// @brief Iterable types that are not strings (string are handled differently
///        for optimization and readability).
template <typename T>
concept Container = !String<T> && Iterable<T>;

/// @brief Match tuples (exists in C++23).
template <typename T>
concept TupleLike = requires(mtf::clean_t<T> obj) {
    std::get<0>(obj);
    typename std::tuple_element_t<0, mtf::clean_t<T>>;
    std::tuple_size_v<mtf::clean_t<T>>;
} && !Array<T>;



}; // namespace serializer::concepts

/// @brief Serializer function type. It should be a concepts but compilers
///        cannot deduce the type of the lambda.
#define SerializerFunction(func, conv)                                         \
    requires {                                                                 \
        func(tools::Context<tools::Phases::Serialization, decltype(conv)>(     \
            conv));                                                            \
    }

#endif
