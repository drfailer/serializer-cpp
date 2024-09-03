#ifndef SERIALIZER_TOOLS_HPP
#define SERIALIZER_TOOLS_HPP
#include "../meta/concepts.hpp"
#include "../meta/type_check.hpp"

namespace serializer::tools {

/******************************************************************************/
/*                                   insert                                   */
/******************************************************************************/

/// @brief Insert an element into an iterable using the insert member function.
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
template <typename Container, typename T>
    requires serializer::concepts::Insertable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.insert(element);
}

/// @brief Insert an element into an iterable using the add member
///        function.
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
template <typename Container, typename T>
    requires serializer::concepts::PushBackable<Container, T>
inline constexpr void insert(Container &&container, T &&element) {
    container.push_back(element);
}

/// @brief Insert an element into an iterable using the operator[].
/// @tparam Container Container type.
/// @tparam T Type of the lement to insert.
/// @param container
/// @param element element to insert in the container.
/// @param idx Index where the element should be inserted in the container.
template <typename Container, typename T>
inline constexpr void insert(Container &&container, T &&element, size_t idx) {
    container[idx] = element;
}

/******************************************************************************/
/*                             tuple manipulation                             */
/******************************************************************************/

/// @brief Remove the first element of a tuple.
/// @tparam H First type of the tuple.
/// @tparam Tyes Other types in the tuple.
/// @tparam Idx  Indicies in the tuple.
/// @param t Tuple.
/// @param index_sequence Indicies of the elements of the tuple.
/// @return Tail of the tuple.
template <typename H, typename... Types, size_t... Idx>
std::tuple<Types...> tuplePopFront_(std::tuple<H, Types...> const &tuple,
                                    std::index_sequence<Idx...>) {
    return std::tuple<Types...>(std::get<Idx + 1>(tuple)...);
}

/// @brief Remove the first element of a tuple.
/// @tparam H First type of the tuple.
/// @tparam Tyes Other types in the tuple.
/// @param t Tuple.
/// @return Tail of the tuple.
template <typename H, typename... Types>
std::tuple<Types...> tuplePopFront(std::tuple<H, Types...> const &tuple) {
    return tuplePopFront_(tuple, std::make_index_sequence<sizeof...(Types)>());
}

/// @brief Multiply the elements in a tuple.
/// @tparam T Output type.
/// @tparam Types Types in the tuple.
/// @tparma Idx Index sequence
/// @param tuple Tuple.
/// @param index_sequence
template <typename T, typename... Types, size_t... Idx>
T tupleProd_(std::tuple<Types...> const &tuple, std::index_sequence<Idx...>) {
    if constexpr (sizeof...(Types) == 0) {
        return 0;
    } else {
        return (std::get<Idx>(tuple) * ...);
    }
}

/// @brief Multiply the elements in a tuple.
/// @tparam T Output type.
/// @tparam Types Types in the tuple.
/// @param tuple Tuple.
template <typename T, typename... Types>
T tupleProd(std::tuple<Types...> const &tuple) {
    return tupleProd_<T>(tuple, std::make_index_sequence<sizeof...(Types)>());
}

/******************************************************************************/
/*                         deserialize with accessors                         */
/******************************************************************************/

/// @brief Deserialize the attribute accessible with `accessor`.
/// @tparam Ser Serializer
/// @tparam T Type of the accessor
/// @param mem Bytes containing the serialized data.
/// @param pos Position in the byte buffer.
/// @param obj Object to deserialize.
/// @param accessor Accessor for the attribute
template <typename Ser, typename T>
constexpr inline void deserializerAccessor(Ser &serializer, auto &obj,
                                           T accessor) {
    if constexpr (mtf::is_setter<T>::value) {
        mtf::setter_arg_type_t<T> elt{};
        serializer.deserialize_(elt);
        std::invoke(accessor, obj, elt);
    } else {
        serializer.deserialize_(std::invoke(accessor, obj));
    }
}

/// @brief Deserialize the attributes accessible with the `accessors`.
/// @tparam Ser Serializer
/// @param mem Bytes containing the serialized data.
/// @param pos Position in the byte buffer.
/// @param obj Object to deserialize.
/// @param accessors Accessors for the attributes
/// @return Position of the next object in the byte buffer.
template <typename Ser>
constexpr inline size_t deserializerAccessors(auto &mem, size_t pos, auto &obj,
                                              auto &&...accessors) {
    Ser serializer(mem, pos);
    ([&, accessors] { deserializerAccessor(serializer, obj, accessors); }(),
     ...);
    return serializer.pos;
}

} // namespace serializer::tools

#endif
