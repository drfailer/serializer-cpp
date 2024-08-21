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

template <typename Conv>
inline constexpr size_t serialize(typename Conv::mem_type &mem, size_t pos,
                                  auto &&...args) {
    [[maybe_unused]] bool first_level = pos == 0;
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (tools::mtf::is_function_v<decltype(args)>) {
                args(Phases::Serialization, conv.mem);
            } else {
                conv.serialize_(args);
            }
        }(),
        ...);
    if constexpr (!tools::mtf::is_vec_v<tools::mtf::base_t<decltype(mem)>>) {
        if (first_level) {
            mem.resize(conv.pos);
        }
    }
    return conv.pos;
}

template <typename Conv>
inline constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos,
                                    auto &&...args) {
    Conv conv(mem, pos);
    (
        [&conv, &args] {
            if constexpr (tools::mtf::is_function_v<decltype(args)>) {
                args(Phases::Deserialization, conv.mem);
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

template <typename Conv, typename T, typename... Supers>
inline constexpr size_t
serialize_super(T const self, typename Conv::mem_type &mem, size_t pos) {
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
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize<serializer::Convertor<decltype(mem)>>(    \
            mem, pos, __VA_ARGS__);                                            \
    }                                                                          \
    constexpr size_t deserialize(auto &mem, size_t pos = 0) {                  \
        return serializer::deserialize<serializer::Convertor<decltype(mem)>>(  \
            mem, pos, __VA_ARGS__);                                            \
    }

#define SERIALIZE_CONV(Conv, ...)                                              \
    size_t serialize(auto &mem, size_t pos = 0) const {                        \
        return serializer::serialize<Conv<decltype(mem)>>(mem, pos,            \
                                                          __VA_ARGS__);        \
    }                                                                          \
    size_t deserialize(auto const &mem, size_t pos = 0) {                      \
        return serializer::deserialize<Conv<decltype(mem)>>(mem, pos,          \
                                                            __VA_ARGS__);      \
    }

#define SERIALIZE_SUPER()

#define SERIALIZE_EMPTY()                                                      \
    size_t serialize(auto &, size_t = 0) const { return 0; }                   \
    size_t deserialize(auto const &, size_t = 0) { return 0; }

// TODO: move this in a proper function and update the macro
#define SERIALIZE_STRUCT()                                                     \
    constexpr size_t serialize(auto &mem, size_t pos = 0) const {              \
        return serializer::serialize_struct(mem, pos, this);                   \
    }                                                                          \
    constexpr size_t deserialize(auto const &mem, size_t pos = 0) {            \
        return serializer::deserialize_struct(mem, pos, this);                 \
    }

#endif
