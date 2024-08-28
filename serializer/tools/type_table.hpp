#ifndef SERIALIZER_TYPE_TABLE_H
#define SERIALIZER_TYPE_TABLE_H
#include "../meta/concepts.hpp"
#include "../meta/type_check.hpp"
#include <stdexcept>
#include <type_traits>

/******************************************************************************/
/*                                 type table                                 */
/******************************************************************************/

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Table that is used to get the identifiers of the serialized types.
/// @tparam T Type of the identifier (positivie number).
/// @tparam Tyes Registers types.
template <concepts::IdType T, typename... Types> struct TypeTable {
    using id_type = T;
};

/* get id *********************************************************************/

/// @brief Get the id of the Target type stored in the given table type.
/// @tparam Target Target type.
/// @tparam T Types of the identifier.
/// @tparam H Firt type in the table.
/// @tparam Ts Rest of the types in the table.
/// @parma _ Type table.
template <typename Target, concepts::IdType T, typename H, typename... Ts>
constexpr inline T getId(TypeTable<T, H, Ts...>) {
    if constexpr (std::is_same_v<Target, H>) {
        return 0;
    } else {
        static_assert(sizeof...(Ts) != 0, "error: type not found in id table.");
        return 1 + getId<Target>(TypeTable<T, Ts...>());
    }
}

/// @brief Get the id of a type from mem at pos.
/// @tparam T Type of the id.
/// @param mem Buffer containing the serialized data.
/// @param pos Start position in the buffer where the id is serialized.
template <typename T> inline constexpr T getId(auto &mem, size_t pos = 0) {
    return *std::bit_cast<const T *>(mem.data() + pos);
}

/* create generci *************************************************************/

/// @brief Helper function for deserializing a generic type.
/// @tparam SuperType Type of the super class.
/// @tparam T Types of the identifier.
/// @tparam H Firt type in the table.
/// @tparam Ts Rest of the types in the table.
/// @param id Identifier of the target type.
/// @parma _ Type table.
/// @parma elt Deserialize element, it will contains the result object.
template <typename SuperType, concepts::IdType T, typename H, typename... Ts>
constexpr inline void createGeneric(T id, TypeTable<T, H, Ts...>, SuperType &elt) {
    if (id == 0) {
        if constexpr (concepts::Pointer<SuperType>) {
            if (elt != nullptr) {
                delete elt;
            }
            elt = new H();
        } else if constexpr (mtf::is_shared_v<SuperType>) {
            elt = std::make_shared<H>();
        } else if constexpr (mtf::is_unique_v<SuperType>) {
            elt = std::make_unique<H>();
        } else {
            static_assert(std::is_copy_assignable_v<SuperType>);
            elt = H();
        }
    } else {
        if constexpr (sizeof...(Ts)) {
            createGeneric(T(id - 1), TypeTable<T, Ts...>(), elt);
        } else {
            throw std::logic_error("error: id not found");
        }
    }
}

/* apply id *******************************************************************/

/// @brief Apply a template lambda to the type with the identifier `id` in the
///        given type table.
/// @tparam IdType Type of the identifiers in the type table.
/// @tparam T First type in the type table.
/// @tparam Ts Rest of the types in the type table.
/// @param id       Identifier of the target type.
/// @param _        Type table.
/// @param function Template lambda / functor to apply on the type. the
///                 operator() should be template parametrized with a type T
///                 that will correspond to the type of identifier id.
template <typename IdType, typename T, typename... Ts>
constexpr void applyId(auto id, serializer::tools::TypeTable<IdType, T, Ts...>,
                       auto function) {
    if (id == 0) {
        function.template operator()<T>();
    } else {
        if constexpr (sizeof...(Ts)) {
            applyId(IdType(id - 1),
                    serializer::tools::TypeTable<IdType, Ts...>(), function);
        } else {
            throw std::logic_error("error: id not found");
        }
    }
}

} // end namespace serializer::tools

#endif
