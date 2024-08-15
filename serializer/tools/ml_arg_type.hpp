#ifndef ML_ARG_TYPE_HPP
#define ML_ARG_TYPE_HPP
#include "../types.hpp"
#include "dynamic_array.hpp"
#include <type_traits>

namespace serializer::tools::mtf {

/// @brief Meta function used to detect the type of the member in the member
///        list. By default, the member list node holds a reference to a member
///        variable. However, we may not want references all the time. For
///        instance, when we want to pass a function or use the dynamic array
///        wrapper type, we don't want to use a reference but a copy.
template <typename T> struct ml_arg_type {
    using type = T &;
};

/// @brief Specialization for functions.
template <> struct ml_arg_type<function_t> {
    using type = function_t;
};

/// @brief Specialization for dynamic array wrapper.
template <typename T, typename DT, typename... DTs>
struct ml_arg_type<serializer::tools::DynamicArray<T, DT, DTs...>> {
    using type = serializer::tools::DynamicArray<T, DT, DTs...>;
};

template <typename T> struct ml_arg_type<serializer::tools::CStruct<T>> {
    using type = serializer::tools::CStruct<T>;
};

/// @brief Shorthand for ml_arg_type.
template <typename T> using ml_arg_type_t = typename ml_arg_type<T>::type;


template <typename T> struct ser_arg_type {
    using type = const T &;
};

/// @brief Specialization for functions.
template <> struct ser_arg_type<function_t> {
    using type = function_t&&;
};

/// @brief Specialization for dynamic array wrapper.
template <typename T, typename DT, typename... DTs>
struct ser_arg_type<serializer::tools::DynamicArray<T, DT, DTs...>> {
    using type = const serializer::tools::DynamicArray<T, DT, DTs...>;
};

template <typename T> struct ser_arg_type<serializer::tools::CStruct<T>> {
    using type = serializer::tools::CStruct<T>;
};

/// @brief Shorthand for ml_arg_type.
template <typename T> using ml_arg_type_t = typename ml_arg_type<T>::type;


template <typename T> struct arg_type {
    using type = T &;
};

/// @brief Specialization for functions.
template <> struct arg_type<function_t> {
    using type = function_t&&;
};

/// @brief Specialization for dynamic array wrapper.
template <typename T, typename DT, typename... DTs>
struct arg_type<serializer::tools::DynamicArray<T, DT, DTs...>> {
    using type = serializer::tools::DynamicArray<T, DT, DTs...>;
};

template <typename T> struct arg_type<serializer::tools::CStruct<T>> {
    using type = serializer::tools::CStruct<T>;
};

/// @brief Shorthand for ml_arg_type.
template <typename T> using arg_type_t = typename arg_type<T>::type;
template <typename T> using ser_arg_type_t = typename ser_arg_type<T>::type;





/// @brief True if the given type T is a dynamic array.
template <typename T> struct is_dynamic_array : std::false_type {};

template <typename T, typename DT, typename... DTs>
struct is_dynamic_array<serializer::tools::DynamicArray<T, DT, DTs...>>
    : std::true_type {};

template <typename T>
constexpr bool is_dynamic_array_v = is_dynamic_array<T>::value;

/// @brief Used to know if the type has to be assigned during the
///        deserialization.
template <typename T> struct not_assigned_on_deserialization {
    static constexpr bool value =
        std::is_array_v<T> || is_dynamic_array_v<T> || is_c_struct_v<T>;
};

template <typename T>
constexpr bool not_assigned_on_deserialization_v =
    not_assigned_on_deserialization<T>::value;


// ---------------------------------------------------------------------------
template <typename ...Types>
struct type_list {
  constexpr type_list() = default;
};

template<typename H>
struct is_type_list : std::false_type {};

template<typename ...Args>
struct is_type_list<type_list<Args...>> : std::true_type {};

} // end namespace serializer::tools::mtf

#endif
