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
template <typename Conv, typename T, typename DT, typename... DTs>
struct ml_arg_type<
    serializer::tools::DynamicArray<Conv, T, DT, DTs...>> {
    using type = serializer::tools::DynamicArray<Conv, T, DT, DTs...>;
};

/// @brief Shorthand for ml_arg_type.
template <typename T> using ml_arg_type_t = typename ml_arg_type<T>::type;

/// @brief True if the given type T is a dynamic array.
template <typename T> struct is_dynamic_array : std::false_type {};

template <typename Conv, typename T, typename DT, typename... DTs>
struct is_dynamic_array<
    serializer::tools::DynamicArray<Conv, T, DT, DTs...>>
    : std::true_type {};

template <typename T>
constexpr bool is_dynamic_array_v = is_dynamic_array<T>::value;

} // end namespace serializer::tools::mtf

#endif