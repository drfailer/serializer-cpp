#ifndef METAFUNCTIONS_HPP
#define METAFUNCTIONS_HPP
#include <memory>
#include <string>
#include <type_traits>

/******************************************************************************/
/*                              transform types                               */
/******************************************************************************/

template <typename T>
using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

template <typename T>
using iter_value_t = typename base_t<T>::iterator::value_type;

/******************************************************************************/
/*                                  pointers                                  */
/******************************************************************************/

template <typename T>
constexpr bool is_concrete_ptr =
    std::is_pointer_v<std::remove_reference_t<T>> &&
    !std::is_polymorphic_v<std::remove_pointer_t<base_t<T>>> &&
    !std::is_abstract_v<std::remove_pointer_t<base_t<T>>>;

/******************************************************************************/
/*                                   string                                   */
/******************************************************************************/

template <typename T>
constexpr bool is_string_v =
    std::is_same_v<std::remove_reference_t<T>, std::string>;

/******************************************************************************/
/*                                  iterator                                  */
/******************************************************************************/

/* non functional yet */
/* template <typename T, typename U = void> */
/* struct is_iterable : std::false_type {}; */

/* template <typename T> */
/* struct is_iterable<T, typename base_t<T>::iterator> : std::true_type {}; */

/* template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value; */

/* template <typename T> */
/* constexpr bool is_deserializable_iterator_v = */
/*     !is_string_v<T> && is_iterable_v<T>; */

/******************************************************************************/
/*                               smart pointers                               */
/******************************************************************************/

/* shared pointers ************************************************************/
template <typename SP> struct is_shared : std::false_type {};

template <typename T> struct is_shared<std::shared_ptr<T>> : std::true_type {};

/* unique pointers ************************************************************/
template <typename SP> struct is_unique : std::false_type {};

template <typename T> struct is_unique<std::unique_ptr<T>> : std::true_type {};

/* smart pointers *************************************************************/
template <typename SP> struct is_smart : std::false_type {};

template <typename T> struct is_smart<std::shared_ptr<T>> : std::true_type {};

template <typename T> struct is_smart<std::unique_ptr<T>> : std::true_type {};

template <typename SP> using is_smart_t = typename is_smart<SP>::type;

/* short hands ****************************************************************/
template <typename SP>
constexpr bool is_smart_ptr_v = std::is_same_v<is_smart_t<SP>, std::true_type>;

template <typename SP>
constexpr bool is_shared_v =
    std::is_same_v<typename is_shared<SP>::type, std::true_type>;

template <typename SP>
constexpr bool is_unique_v =
    std::is_same_v<typename is_unique<SP>::type, std::true_type>;

/******************************************************************************/
/*                                   pairs                                    */
/******************************************************************************/

template <typename T> struct is_pair {
    static const bool value = false;
};

template <typename T1, typename T2> struct is_pair<std::pair<T1, T2>> {
    static const bool value = true;
};

template <typename T> constexpr bool is_pair_v = is_pair<T>::value;

/******************************************************************************/
/*                              is serializable                               */
/******************************************************************************/

template <typename T, typename U = void>
struct is_deserializable : std::false_type {};

template <typename T>
struct is_deserializable<T, decltype(std::declval<T>().deserialize(""))>
    : std::true_type {};

template <typename T>
constexpr bool is_deserializable_v = is_deserializable<T>::value;

#endif
