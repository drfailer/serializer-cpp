#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convert.hpp"
#include "convertor/convertor.hpp"
#include "tools/concepts.hpp"
#include "tools/ml_arg_type.hpp"
#include "types.hpp"
#include <string>

namespace serializer {

template <typename Conv, typename... Args>
constexpr void serialize(std::string &str, tools::mtf::type_list<Args...>,
                         tools::mtf::ser_arg_type_t<Args>... args) {
    (
        [&str, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Serialization, str);
            } else {
                Conv().serialize_(args, str);
            }
        }(),
        ...);
}

template <typename Conv, typename H, typename... Args>
requires (!tools::mtf::is_type_list<H>::value)
constexpr void serialize(std::string &str, const H &h, const Args &...args) {
    serialize<Conv>(str, tools::mtf::type_list<H, Args...>(), h, args...);
}

template <typename Conv, typename... Args>
constexpr void deserialize(std::string_view &str,
                           tools::mtf::type_list<Args...>,
                           tools::mtf::arg_type_t<Args>... args) {
    (
        [&str, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Deserialization, str);
            } else if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                     Args> ||
                                 tools::concepts::Deserializable<Args>) {
                // for the types that can't be assigned
                Conv().deserialize_(str, args);
            } else if constexpr (std::is_move_assignable_v<Args>) {
                // for the types that have to be assigned
                args = std::move(Conv().deserialize_(str, args));
            } else {
                static_assert(std::is_copy_assignable_v<Args>,
                              "deserialized types must be assignable.");
                args = Conv().deserialize_(str, args);
            }
        }(),
        ...);
}

template <typename Conv, typename H, typename... Args>
requires (!tools::mtf::is_type_list<H>::value)
constexpr void deserialize(std::string_view &str, H &h, Args &...args) {
    deserialize<Conv>(str, tools::mtf::type_list<H, Args...>(), h, args...);
}

} // end namespace serializer

#define HELPER_serialize                                                       \
    std::string serialize() const {                                            \
        std::string str;                                                       \
        serialize(str);                                                        \
        return str;                                                            \
    }

#define HELPER_deserialize                                                     \
    void deserialize(std::string &str) {                                       \
        std::string_view strv = str;                                           \
        deserialize(strv);                                                     \
    }

#define SERIALIZE(...)                                                         \
    void serialize(std::string &str) const {                                   \
        serializer::serialize<serializer::Convertor<>>(str, __VA_ARGS__);      \
    }                                                                          \
    void deserialize(std::string_view &str) {                                  \
        serializer::deserialize<serializer::Convertor<>>(str, __VA_ARGS__);    \
    }                                                                          \
    HELPER_serialize HELPER_deserialize

#define SERIALIZE_STRUCT()                                                     \
    void serialize(std::string &str) const {                                   \
        str.append(std::bit_cast<const char *>(this), sizeof(*this));          \
    }                                                                          \
    constexpr void deserialize(std::string_view &str) {                        \
        *this = *std::bit_cast<decltype(this)>(str.data());                    \
        str = str.substr(sizeof(*this));                                       \
    }                                                                          \
    HELPER_serialize HELPER_deserialize

#endif
