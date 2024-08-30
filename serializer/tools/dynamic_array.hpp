#ifndef SERIALIZER_DYNAMIC_ARRAY_HPP
#define SERIALIZER_DYNAMIC_ARRAY_HPP
#include "../meta/concepts.hpp"

/******************************************************************************/
/*                               Dynamic Array                                */
/******************************************************************************/

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Wrapper object for dynamic arrays (can store references to the
///        variables that contains the array size).
template <concepts::Pointer T, typename... DTs> struct DynamicArray {
    /// @brief Constructor that should be used by the user.
    /// @param mem Reference to the pointer of the array that should be
    ///            serialized.
    /// @params dim Reference or value of the first dimension of the array (the
    ///         array must have at least one dimension)
    /// @params dims Other dimensions of the array.
    constexpr explicit DynamicArray(T &mem, DTs const &...dims)
        : mem(mem), dimensions(dims...) {}

    /// @brief Constructor that is used to create sub-arrays in the default
    ///        convertor.
    /// @param mem Reference to the pointer of the array that should be
    ///            serialized.
    /// @param dimensions Tuple that holds the dimensions of the sub-array.
    constexpr explicit DynamicArray(T &mem,
                                    std::tuple<const DTs &...> &&dimensions)
        : mem(mem), dimensions(dimensions) {}

    T &mem; ///< reference to the pointer of the array.
    std::tuple<const DTs &...> dimensions; ///< dimensions of the array.
};

} // end namespace serializer::tools

#endif
