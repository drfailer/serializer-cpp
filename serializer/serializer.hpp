#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP
#include "tools/concepts.hpp"
#include "tools/metafunctions.hpp"
#include <cstddef>
#include <tuple>

namespace serializer {

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

template <typename Conv, typename... Args> struct serializer {
    constexpr serializer(Args &&...args) : members(args...) {}

    constexpr size_t serialize(auto &mem, size_t pos) {
        return serialize(mem, pos, std::make_index_sequence<sizeof...(Args)>());
    }

    constexpr size_t deserialize(typename Conv::mem_type &mem, size_t pos) {
        return deserialize(mem, pos,
                           std::make_index_sequence<sizeof...(Args)>());
    }

  private:
    template <size_t... Idx>
    constexpr size_t serialize(auto &mem, size_t pos,
                               std::index_sequence<Idx...>) {
        [[maybe_unused]] bool first_level = pos == 0;
        Conv conv(mem, pos);
        (
            [this, &conv] {
                if constexpr (SerializerFunction(std::get<Idx>(members),
                                                 conv)) {
                    std::get<Idx>(members).operator()<Phases::Serialization>(
                        conv);
                } else {
                    conv.serialize_(std::get<Idx>(members));
                }
            }(),
            ...);
        if constexpr (!tools::mtf::is_vec_v<
                          tools::mtf::base_t<decltype(mem)>>) {
            if (first_level) [[unlikely]] {
                mem.resize(conv.pos);
            }
        }
        return conv.pos;
    }

    template <size_t... Idx>
    constexpr size_t deserialize(auto &mem, size_t pos,
                                 std::index_sequence<Idx...>) {
        Conv conv(mem, pos);
        (
            [this, &conv] {
                if constexpr (SerializerFunction(std::get<Idx>(members),
                                                 conv)) {
                    std::get<Idx>(members).operator()<Phases::Deserialization>(
                        conv);
                } else {
                    conv.deserialize_(std::get<Idx>(members));
                }
            }(),
            ...);
        return conv.pos;
    }

  private:
    std::tuple<Args &&...> members;
};

} // namespace serializer

#endif
