#ifndef SERIALIZER_ID_TABLE_H
#define SERIALIZER_ID_TABLE_H
#include "serializer/meta/concepts.hpp"
#include "serializer/meta/type_check.hpp"
#include <stdexcept>
#include <type_traits>

namespace serializer::tools {

template <typename IdType, typename... Value> struct IdTable {
    using id_type = IdType;
};

template <typename T, typename IdType, typename Value, typename... Values>
constexpr IdType getId(IdTable<IdType, Value, Values...>) {
    if constexpr (std::is_same_v<T, Value>) {
        return 0;
    } else {
        static_assert(sizeof...(Values) != 0,
                      "error: type not found in id table.");
        return 1 + getId<T>(IdTable<IdType, Values...>());
    }
}

template <typename Result, typename IdType, typename Value, typename... Values>
constexpr void createGeneric(IdType id, IdTable<IdType, Value, Values...>,
                             Result &elt) {
    if (id == 0) {
        if constexpr (concepts::Pointer<Result>) {
            if (elt != nullptr) {
                delete elt;
            }
            elt = new Value();
        } else if constexpr (mtf::is_shared_v<Result>) {
            elt = std::make_shared<Value>();
        } else if constexpr (mtf::is_unique_v<Result>) {
            elt = std::make_unique<Value>();
        } else {
            static_assert(std::is_copy_assignable_v<Result>);
            elt = Value();
        }
    } else {
        if constexpr (sizeof...(Values)) {
            createGeneric(id - 1, IdTable<IdType, Values...>(), elt);
        } else {
            throw std::logic_error("error: id not found");
        }
    }
}

template <typename IdType, typename T, typename... TS>
constexpr void applyId(auto id,
                           serializer::tools::IdTable<IdType, T, TS...>,
                           auto function) {
    if (id == 0) {
        function.template operator()<T>();
    } else {
        if constexpr (sizeof...(TS)) {
            applyId(id - 1, serializer::tools::IdTable<IdType, TS...>(),
                        function);
        } else {
            throw std::logic_error("error: id not found");
        }
    }
}

} // end namespace serializer::tools

#endif
