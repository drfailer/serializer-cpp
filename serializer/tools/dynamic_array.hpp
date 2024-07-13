#ifndef DYNAMIC_ARRAY_HPP
#define DYNAMIC_ARRAY_HPP
#include "concepts.hpp"

/******************************************************************************/
/*                               Dynamic Array                                */
/******************************************************************************/

namespace serializer::tools {

/// @brief Wrapper object for dynamic arrays (can store references to the
///        variables that contains the array size).
template <typename Conv, concepts::Pointer T, typename DT, typename... DTs>
struct DynamicArray {
    /// @brief Constructor that should be used by the user.
    /// @param mem Reference to the pointer of the array that should be
    ///            serialized.
    /// @params dim Reference or value of the first dimension of the array (the
    ///         array must have at least one dimension)
    /// @params dims Other dimensions of the array.
    explicit DynamicArray(T &mem, DT dim, DTs... dims)
        : mem(mem), dimensions(dim, dims...) {}

    /// @brief Constructor that is used to create sub-arrays in the default
    ///        convertor.
    /// @param mem Reference to the pointer of the array that should be
    ///            serialized.
    /// @param dimensions Tuple that holds the dimensions of the sub-array.
    explicit DynamicArray(T &mem, std::tuple<DT, DTs...> dimensions)
        : mem(mem), dimensions(dimensions) {}

    T &mem; ///< reference to the pointer of the array.
    std::tuple<DT, DTs...> dimensions; ///< dimensions of the array.
};

} // end namespace serializer::tools

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

/// @brief Helper macro that can be used to avoid writing all the namespaces.
/// @param ArrType Type of the pointer to serialize (int* for a basic array).
/// @param ... Types of the dimensions (ex: size_t if we want to pass it by
///            value or size_t& by reference)
#define SER_DARR_T(ArrType, ...)                                               \
    serializer::tools::DynamicArray<serializer::Convertor<>, ArrType,          \
                                    __VA_ARGS__>

#endif
