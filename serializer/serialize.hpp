#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convert.hpp"
#include "convertor/convertor.hpp"
#include "tools/concepts.hpp"
#include "tools/ml_arg_type.hpp"
#include "types.hpp"

namespace serializer {

template <typename Conv, typename... Args>
constexpr void serialize(typename Conv::mem_type &mem,
                         tools::mtf::type_list<Args...>,
                         tools::mtf::ser_arg_type_t<Args>... args) {
    Conv conv(mem);
    (
        [&conv, &mem, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Serialization, mem);
            } else {
                conv.serialize_(args);
            }
        }(),
        ...);
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr void serialize(typename Conv::mem_type &mem, const H &h,
                         const Args &...args) {
    serialize<Conv>(mem, tools::mtf::type_list<H, Args...>(), h, args...);
}

template <typename Conv, typename... Args>
constexpr void deserialize(typename Conv::mem_type &mem,
                           tools::mtf::type_list<Args...>,
                           tools::mtf::arg_type_t<Args>... args) {
    Conv conv(mem);
    (
        [&conv, &mem, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Deserialization, mem);
            } else if constexpr (tools::mtf::not_assigned_on_deserialization_v<
                                     Args> ||
                                 tools::concepts::Deserializable<Args>) {
                // for the types that can't be assigned
                conv.deserialize_(mem, args);
            } else if constexpr (std::is_move_assignable_v<Args>) {
                // for the types that have to be assigned
                args = std::move(conv.deserialize_(mem, args));
            } else {
                static_assert(std::is_copy_assignable_v<Args>,
                              "deserialized types must be assignable.");
                args = conv.deserialize_(mem, args);
            }
        }(),
        ...);
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr void deserialize(typename Conv::mem_type &mem, H &h, Args &...args) {
    deserialize<Conv>(mem, tools::mtf::type_list<H, Args...>(), h, args...);
}

using default_mem_type = std::vector<std::byte>;

} // end namespace serializer

#define HELPER_serialize                                                       \
    serializer::default_mem_type serialize() const {                           \
        serializer::default_mem_type mem;                                      \
        serialize<serializer::default_mem_type>(mem);                          \
        return mem;                                                            \
    }

#define SERIALIZE(...)                                                         \
    template <typename MemT> void serialize(MemT &mem) const {                 \
        serializer::serialize<serializer::Convertor<MemT>>(mem, __VA_ARGS__);  \
    }                                                                          \
    template <typename MemT> void deserialize(MemT &mem) {                     \
        serializer::deserialize<serializer::Convertor<MemT>>(mem,              \
                                                             __VA_ARGS__);     \
    }                                                                          \
    HELPER_serialize

#define SERIALIZE_STRUCT()                                                     \
    template <typename MemT> void serialize(MemT &mem) const {                 \
        mem.append(std::bit_cast<const char *>(this), sizeof(*this));          \
    }                                                                          \
    template <typename MemT> constexpr void deserialize(MemT &mem) {           \
        *this = *std::bit_cast<decltype(this)>(mem.data());                    \
        mem = mem.substr(sizeof(*this));                                       \
    }                                                                          \
    HELPER_serialize

#endif
