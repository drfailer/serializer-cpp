#ifndef SERIALIZER_CONVERT_HPP
#define SERIALIZER_CONVERT_HPP
#include <string>

namespace serializer {

/// @brief Used to create a convert behavior for a specific type. This class is
///        used when we want to create a custom convertor that add support for
///        external types.
/// @param T Type for which we want to add the convert behavior for.
template <typename T> struct Convert {
    virtual void serialize(T const &, std::string &) const = 0;
    virtual T deserialize(std::string_view &, T &) = 0;
};

}

#endif // SERIALIZER_CONVERT_H
