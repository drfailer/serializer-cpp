#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convert.hpp"
#include "convertor/convertor.hpp"
#include "tools/concepts.hpp"
#include "tools/ml_arg_type.hpp"
#include "tools/vec.hpp"
#include "types.hpp"

// TODO: use always inline attribute

namespace serializer {

template <typename Conv, typename... Args>
inline constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos,
                           Args &&...args) {
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
inline constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos,
                             Args &&...args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (tools::mtf::is_function_v<Args>) {
                args(Phases::Deserialization, conv.mem);
            } else {
                // for the types that can't be assigned
                conv.deserialize_(args);
            }
        }(),
        ...);
    return conv.pos;
}

template <typename MemT, typename T>
inline constexpr size_t serialize_struct(MemT &mem, size_t pos, T const obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    mem.append(pos, std::bit_cast<const byte_type *>(obj), nb_bytes);
    return pos + nb_bytes;
}

template <typename MemT, typename T>
inline constexpr size_t deserialize_struct(MemT &mem, size_t pos, T const obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

template <typename Conv, typename T, typename... Supers>
inline constexpr size_t serialize_super(T const self, typename Conv::mem_type &mem,
                                 size_t pos) {
    ([&self, &mem,
      &pos] { pos = dynamic_cast<Supers>(self).serialize(mem, pos); }(),
     ...);
    return pos;
}

template <typename Conv, typename T, typename... Supers>
inline constexpr size_t deserialize_super(T self, typename Conv::mem_type &mem,
                                   size_t pos) {
    ([&self, &mem,
      &pos] { pos = dynamic_cast<Supers>(self).deserialize(mem, pos); }(),
     ...);
    return pos;
}

using default_mem_type = tools::vec<uint8_t>;
/* using default_mem_type = std::vector<uint8_t>; */

} // end namespace serializer

#define SERIALIZE(...)                                                         \
    template <typename MemT>                                                   \
    constexpr size_t serialize(MemT &mem, size_t pos = 0) const {              \
        return serializer::serialize<serializer::Convertor<MemT>>(             \
            mem, pos, __VA_ARGS__);                                            \
    }                                                                          \
    template <typename MemT>                                                   \
    constexpr size_t deserialize(MemT const &mem, size_t pos = 0) {            \
        return serializer::deserialize<serializer::Convertor<const MemT>>(     \
            mem, pos, __VA_ARGS__);                                            \
    }

#define SERIALIZE_CONV(Conv, ...)                                              \
    template <typename MemT>                                                   \
    size_t serialize(MemT &mem, size_t pos = 0) const {                        \
        return serializer::serialize<Conv<MemT>>(mem, pos, __VA_ARGS__);       \
    }                                                                          \
    template <typename MemT>                                                   \
    size_t deserialize(MemT const &mem, size_t pos = 0) {                      \
        return serializer::deserialize<Conv<const MemT>>(mem, pos,             \
                                                         __VA_ARGS__);         \
    }

#define SERIALIZE_SUPER()

#define SERIALIZE_EMPTY()                                                      \
    template <typename MemT> size_t serialize(MemT &, size_t = 0) const {      \
        return 0;                                                              \
    }                                                                          \
    template <typename MemT> size_t deserialize(MemT const &, size_t = 0) {    \
        return 0;                                                              \
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
