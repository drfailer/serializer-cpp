#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convert.hpp"
#include "convertor/convertor.hpp"
#include "tools/concepts.hpp"
#include "tools/ml_arg_type.hpp"
#include "types.hpp"

namespace serializer {

template <typename Conv, typename... Args>
constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos,
                           tools::mtf::type_list<Args...>,
                           tools::mtf::ser_arg_type_t<Args>... args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Serialization, conv.mem);
            } else {
                conv.serialize_(args);
            }
        }(),
        ...);
    return conv.spos;
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos, const H &h,
                           const Args &...args) {
    return serialize<Conv>(mem, pos, tools::mtf::type_list<H, Args...>(), h,
                           args...);
}

template <typename Conv, typename... Args>
constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos,
                             tools::mtf::type_list<Args...>,
                             tools::mtf::arg_type_t<Args>... args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Deserialization, conv.mem);
            } else if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                     Args> ||
                                 tools::concepts::Deserializable<Args>) {
                // for the types that can't be assigned
                conv.deserialize_(args);
            } else if constexpr (std::is_move_assignable_v<Args>) {
                // for the types that have to be assigned
                args = std::move(conv.deserialize_(args));
            } else {
                static_assert(std::is_copy_assignable_v<Args>,
                              "deserialized types must be assignable.");
                args = conv.deserialize_(args);
            }
        }(),
        ...);
    return conv.dpos;
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos, H &h,
                             Args &...args) {
    return deserialize<Conv>(mem, pos, tools::mtf::type_list<H, Args...>(), h,
                             args...);
}

using default_mem_type = std::vector<uint8_t>;

} // end namespace serializer

#define HELPER_serialize                                                       \
    serializer::default_mem_type serialize() const {                           \
        serializer::default_mem_type mem;                                      \
        serialize<serializer::default_mem_type>(mem);                          \
        return mem;                                                            \
    }

#define SERIALIZE(...)                                                         \
    template <typename MemT>                                                   \
    size_t serialize(MemT &mem, size_t pos = 0) const {                        \
        return serializer::serialize<serializer::Convertor<MemT>>(             \
            mem, pos, __VA_ARGS__);                                            \
    }                                                                          \
    template <typename MemT>                                                   \
    size_t deserialize(MemT const &mem, size_t pos = 0) {                      \
        return serializer::deserialize<serializer::Convertor<const MemT>>(     \
            mem, pos, __VA_ARGS__);                                            \
    }                                                                          \
    HELPER_serialize

#define SERIALIZE_STRUCT()                                                     \
    template <typename MemT>                                                   \
    size_t serialize(MemT &mem, size_t pos = 0) const {                        \
        mem.resize(sizeof(*this));                                             \
        std::memcpy(mem.data(), std::bit_cast<const char *>(this),             \
                    sizeof(*this));                                            \
        return pos + sizeof(*this);                                            \
    }                                                                          \
    template <typename MemT>                                                   \
    constexpr void deserialize(MemT const &mem, size_t pos = 0) {              \
        *this = *std::bit_cast<const decltype(this)>(mem.data());              \
        mem = mem.substr(sizeof(*this));                                       \
        return pos + sizeof(*this);                                            \
    }                                                                          \
    HELPER_serialize

#endif
