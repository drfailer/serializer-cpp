#ifndef METAFUNCTIONS_HPP
#define METAFUNCTIONS_HPP
#include <memory>
#include <string>
#include <type_traits>

namespace serializer::mtf {

/******************************************************************************/
/*                              transform types                               */
/******************************************************************************/

template <typename T>
using base_t = typename std::remove_const_t<std::remove_reference_t<T>>;

template <typename T> struct iter_value {
    using type = typename base_t<T>::iterator::value_type;
};

template <typename T, size_t N> struct iter_value<std::array<T, N>> {
    using type = T;
};

template <typename T> using iter_value_t = iter_value<T>::type;

/******************************************************************************/
/*                                  pointers                                  */
/******************************************************************************/

template <typename T>
constexpr bool is_concrete_ptr_v =
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
/*                                   tuples                                   */
/******************************************************************************/

template <typename H, typename T> struct tuple_push_front;

template <typename H, template <typename...> class T, typename... Types>
struct tuple_push_front<H, T<Types...>> {
    using type = T<H, Types...>;
};

template <typename H, typename T>
using tuple_push_front_t = typename tuple_push_front<H, T>::type;

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

template <typename T>
struct is_std_array : std::false_type {};

template <typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_std_array_v = is_std_array<T>::value;

} // namespace serializer::mtf

#endif
