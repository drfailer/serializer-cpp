#ifndef METAFUNCTIONS_HPP
#define METAFUNCTIONS_HPP
#include "../types.hpp"
#include "../tools/vec.hpp"
#include <memory>
#include <string>
#include <type_traits>

namespace serializer::tools::mtf {

/******************************************************************************/
/*                              transform types                               */
/******************************************************************************/

/// @brief Removes the reference and the const on a type.
template <typename T>
using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

/// @brief Allow easy access to the type of the elements_ in a members_.
///        We don't use value_type directly in case of a not well implemented
///        members_. Though, we assume that the iterator respects the standard.
template <typename T> struct iter_value {
    using type = typename base_t<T>::iterator::value_type;
};

/// @brief Specific case for std::array (it is iterable using pointers and not
///        proper iterators).
template <typename T, size_t N> struct iter_value<std::array<T, N>> {
    using type = T;
};

/// @brief iter_value shorthand.
template <typename T> using iter_value_t = iter_value<T>::type;

/// @brief Check if a type T is in the parameter pac Types. value is true if T
///        is in Types.
template <typename T, typename... Types> struct contains;
template <typename T> struct contains<T> {
    static constexpr bool value = false;
};
template <typename T, typename H, typename... Tail>
struct contains<T, H, Tail...> {
    static constexpr bool value =
        std::is_same_v<T, H> || contains<base_t<T>, Tail...>::value;
};

/// @brief Shorthand for contains.
template <typename T, typename... Types>
constexpr bool contains_v = contains<base_t<T>, Types...>::value;

/******************************************************************************/
/*                                  pointers                                  */
/******************************************************************************/

/// @brief Check if a pointer is concrete or not.
template <typename T>
constexpr bool is_concrete_ptr_v =
    std::is_pointer_v<std::remove_reference_t<T>> &&
    !std::is_polymorphic_v<std::remove_pointer_t<base_t<T>>> &&
    !std::is_abstract_v<std::remove_pointer_t<base_t<T>>>;

/******************************************************************************/
/*                                   string                                   */
/******************************************************************************/

/// @brief Check if a type T is a string / const string.
template <typename T>
constexpr bool is_string_v =
    std::is_same_v<std::remove_reference_t<T>, std::string>;

/******************************************************************************/
/*                               smart pointers                               */
/******************************************************************************/

/* shared pointers ************************************************************/

/// @brief Checks if a type SP is a shared_ptr
template <typename SP> struct is_shared : std::false_type {};

template <typename T> struct is_shared<std::shared_ptr<T>> : std::true_type {};

template <typename SP> constexpr bool is_shared_v = is_shared<SP>::value;

/* unique pointers ************************************************************/

/// @brief Checks if a type SP is a unique_ptr
template <typename SP> struct is_unique : std::false_type {};

template <typename T> struct is_unique<std::unique_ptr<T>> : std::true_type {};

template <typename SP> constexpr bool is_unique_v = is_unique<SP>::value;

/* smart pointers *************************************************************/

/// @brief Checks if a type is a shared or a unique_ptr
template <typename SP>
constexpr bool is_smart_ptr_v = is_shared_v<SP> || is_unique_v<SP>;

/******************************************************************************/
/*                                   tuples                                   */
/******************************************************************************/

/// @brief Put the type H at the beginning of the tuple T
template <typename H, typename T> struct tuple_push_front;

template <typename H, template <typename...> class T, typename... Types>
struct tuple_push_front<H, T<Types...>> {
    using type = T<H, Types...>;
};

template <typename H, typename T>
using tuple_push_front_t = typename tuple_push_front<H, T>::type;

/// @brief Remove const specifier on the types contained in the tuple T.
template <typename T> struct remove_const_tuple;

template <template <typename...> class T> struct remove_const_tuple<T<>> {
    using type = T<>;
};

template <template <typename...> class T, typename H, typename... Types>
struct remove_const_tuple<T<H, Types...>> {
    using type =
        tuple_push_front_t<std::remove_const_t<H>,
                           typename remove_const_tuple<T<Types...>>::type>;
};

template <typename T1, typename T2>
struct remove_const_tuple<std::pair<T1, T2>> {
    using type = std::pair<std::remove_const_t<T1>, std::remove_const_t<T2>>;
};

template <typename T>
using remove_const_tuple_t = typename remove_const_tuple<T>::type;

/******************************************************************************/
/*                                   array                                    */
/******************************************************************************/

/// @brief Checks if the type T is an std::array.
template <typename T> struct is_std_array : std::false_type {};

template <typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T> constexpr bool is_std_array_v = is_std_array<T>::value;

/// @brief True if T is a vec
template <typename T> struct is_vec : std::false_type {};

template <typename T>
struct is_vec<serializer::tools::vec<T>> : std::true_type {};

template <typename T>
constexpr bool is_vec_v = is_vec<T>::value;

} // namespace serializer::tools::mtf

#endif
