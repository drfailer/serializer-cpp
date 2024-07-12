#ifndef TYPES_HPP
#define TYPES_HPP
#include <functional>
#include <string_view>

namespace serializer {
    enum class Phases {
        Serialization,
        Deserialization
    };

    using function_t = std::function<void(Phases, std::string_view const &)>;
} // end namespace serializer

#endif
