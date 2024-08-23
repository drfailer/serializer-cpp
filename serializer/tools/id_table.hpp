#ifndef SERIALIZER_ID_TABLE_H
#define SERIALIZER_ID_TABLE_H
#include <stdexcept>
#include <type_traits>

namespace serializer::tools {

template <typename IdType, typename... Value> struct id_table {};

template <typename T, typename IdType, typename Value, typename... Values>
constexpr IdType get_id(id_table<IdType, Value, Values...>) {
    if constexpr (std::is_same_v<T, Value>) {
        return 0;
    } else {
        static_assert(sizeof...(Values) != 0,
                      "error: type not found in id table.");
        return 1 + get_id<T>(id_table<IdType, Values...>());
    }
}

template <typename Result, typename IdType, typename Value, typename... Values>
constexpr void create_with_id(Result &elt, id_table<IdType, Value, Values...>,
                              IdType id) {
    if (id == 0) {
        if (elt != nullptr) {
            delete elt;
        }
        elt = new Value();
    } else {
        if constexpr (sizeof...(Values)) {
          create_with_id(elt, id_table<IdType, Values...>(), id - 1);
        } else {
          throw std::logic_error("error: id not found");
        }
    }
}

} // end namespace serializer::tools

#endif
