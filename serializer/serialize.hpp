#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convert.hpp"
#include "convertor/convertor.hpp"
#include "tools/concepts.hpp"
#include "tools/ml_arg_type.hpp"
#include "tools/vec.hpp"
#include "types.hpp"

namespace serializer {

template <typename Conv, typename... Args>
constexpr size_t serialize_(typename Conv::mem_type &mem, size_t pos,
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
    return conv.pos;
}

template <typename Conv, typename... Args>
constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos,
                           tools::mtf::type_list<Args...>,
                           tools::mtf::ser_arg_type_t<Args>... args) {
  return serialize_<Conv, Args...>(mem, pos, args...);
}

template <typename Conv, typename... Args>
constexpr size_t deserialize_(typename Conv::mem_type &mem, size_t pos,
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
    return conv.pos;
}

template <typename Conv, typename... Args>
constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos,
                             tools::mtf::type_list<Args...>,
                             tools::mtf::arg_type_t<Args>... args) {
  return deserialize_<Conv, Args...>(mem, pos, args...);
}

template <typename MemT, typename T>
constexpr size_t serialize_struct(MemT &mem, size_t pos, T const *obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    mem.append(pos, std::bit_cast<const byte_type *>(obj), nb_bytes);
    return pos + nb_bytes;
}

template <typename MemT, typename T>
constexpr size_t deserialize_struct(MemT &mem, size_t pos, T const obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos, const H &h,
                           const Args &...args) {
    return serialize<Conv>(mem, pos, tools::mtf::type_list<H, Args...>(), h,
                           args...);
}

template <typename Conv, typename H, typename... Args>
    requires(!tools::mtf::is_type_list<H>::value)
constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos, H &h,
                             Args &...args) {
    return deserialize<Conv>(mem, pos, tools::mtf::type_list<H, Args...>(), h,
                             args...);
}

using default_mem_type = tools::vec<uint8_t>;
/* using default_mem_type = std::vector<uint8_t>; */

} // end namespace serializer

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
    }

// TODO: move this in a proper function and update the macro
#define SERIALIZE_STRUCT()                                                     \
    template <typename MemT>                                                   \
    constexpr size_t serialize(MemT &mem, size_t pos = 0) const {              \
        return serializer::serialize_struct(mem, pos, this);                   \
    }                                                                          \
    template <typename MemT>                                                   \
    constexpr size_t deserialize(MemT const &mem, size_t pos = 0) {            \
        return serializer::deserialize_struct(mem, pos, this);                 \
    }

#endif
