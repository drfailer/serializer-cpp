#ifndef ML_ARG_TYPE_HPP
#define ML_ARG_TYPE_HPP
#include "../types.hpp"
#include "dynamic_array.hpp"
#include <type_traits>

namespace serializer::tools::mtf {

template <typename T> struct ml_arg_type {
    using type = T &;
};

template <> struct ml_arg_type<function_t> {
    using type = function_t;
};

template <typename Conv, typename T, typename DT, typename... DTs>
struct ml_arg_type<
    serializer::tools::DynamicArray<Conv, T, DT, DTs...>> {
    using type = serializer::tools::DynamicArray<Conv, T, DT, DTs...>;
};

template <typename T> using ml_arg_type_t = typename ml_arg_type<T>::type;

template <typename T> struct is_dynamic_array : std::false_type {};

template <typename Conv, typename T, typename DT, typename... DTs>
struct is_dynamic_array<
    serializer::tools::DynamicArray<Conv, T, DT, DTs...>>
    : std::true_type {};

template <typename T>
constexpr bool is_dynamic_array_v = is_dynamic_array<T>::value;

} // end namespace serializer::tools::mtf

#endif
