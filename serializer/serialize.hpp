#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "convertor/convertor.hpp"
#include "meta/concepts.hpp"
#include "tools/context.hpp"
#include "tools/vec.hpp"

// TODO: use always inline attribute

namespace serializer {

template <typename Conv>
inline constexpr size_t serialize(auto &mem, size_t pos, auto &&...args) {
    using mem_t = decltype(mem);
    [[maybe_unused]] bool first_level = pos == 0;

    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (SerializerFunction(args, conv)) {
                args(Context<Phases::Serialization, decltype(conv)>(conv));
            } else {
                conv.serialize_(args);
            }
        }(),
        ...);
    if constexpr (!mtf::is_vec_v<mem_t> && concepts::Resizeable<mem_t>) {
        if (first_level) [[unlikely]] {
            mem.resize(conv.pos);
        }
    }
    return conv.pos;
}

template <typename Conv>
inline constexpr size_t deserialize(auto &mem, size_t pos, auto &&...args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (SerializerFunction(args, conv)) {
                args(Context<Phases::Deserialization, decltype(conv)>(conv));
            } else {
                conv.deserialize_(args);
            }
        }(),
        ...);
    return conv.pos;
}

template <typename T>
inline constexpr size_t serialize_struct(auto &mem, size_t pos, T const obj) {
    constexpr size_t nb_bytes = sizeof(*obj);
    Convertor<decltype(mem)> conv(mem, pos);
    using byte_type = std::remove_cvref_t<decltype(mem[0])>;
    conv.append(std::bit_cast<const byte_type *>(obj), nb_bytes);
    return conv.pos;
}

template <typename T>
inline constexpr size_t deserialize_struct(auto &mem, size_t pos, T const obj) {
    *obj = *std::bit_cast<const decltype(obj)>(mem.data() + pos);
    return pos + sizeof(*obj);
}

using default_mem_type = tools::vec<uint8_t>;
/* using default_mem_type = std::vector<uint8_t>; */

template <typename Super> inline constexpr Super &super(auto *obj) {
    return *static_cast<Super *>(obj);
}

template <typename Super> inline constexpr Super const &super(auto const *obj) {
    return *static_cast<Super const *>(obj);
}

} // end namespace serializer

#define SERIALIZE_CONV(Conv, ...)                                              \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize<Conv<decltype(mem)>>(mem, pos,            \
                                                          __VA_ARGS__);        \
    }                                                                          \
    constexpr size_t deserialize(auto &mem, size_t pos = 0) {                  \
        return serializer::deserialize<Conv<decltype(mem)>>(mem, pos,          \
                                                            __VA_ARGS__);      \
    }

#define SERIALIZE(...) SERIALIZE_CONV(serializer::Convertor, __VA_ARGS__)

#define __SERIALIZE__(Conv, virt, over, ...)                                   \
    constexpr virt size_t serialize(typename Conv::mem_type &mem,              \
                                    size_t pos = 0) const over {               \
        return serializer::serialize<Conv>(mem, pos, __VA_ARGS__);             \
    }                                                                          \
    constexpr virt size_t deserialize(typename Conv::mem_type &mem,            \
                                      size_t pos = 0) over {                   \
        return serializer::deserialize<Conv>(mem, pos, __VA_ARGS__);           \
    }

#define VIRTUAL_SERIALIZE(Conv, ...)                                           \
    __SERIALIZE__(Conv, virtual, /* over */, __VA_ARGS__)

#define SERIALIZE_OVERRIDE(Conv, ...)                                          \
    __SERIALIZE__(Conv, /* virt */, override, __VA_ARGS__)

#define SERIALIZE_EMPTY()                                                      \
    constexpr virtual size_t serialize(auto &, size_t = 0) const { return 0; } \
    constexpr virtual size_t deserialize(auto const &, size_t = 0) { return 0; }

#define SERIALIZE_ABSTRACT()                                                   \
    constexpr virtual size_t serialize(auto &, size_t = 0) const = 0;          \
    constexpr virtual size_t deserialize(auto const &, size_t = 0) = 0;

#define SERIALIZE_STRUCT()                                                     \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize_struct(mem, pos, this);                   \
    }                                                                          \
    constexpr size_t deserialize(auto const &mem, size_t pos = 0) {            \
        return serializer::deserialize_struct(mem, pos, this);                 \
    }

#endif
