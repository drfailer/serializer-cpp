#ifndef SERIALIZER_TYPE_TABLE_H
#define SERIALIZER_TYPE_TABLE_H
#include "../meta/concepts.hpp"
#include "../meta/type_check.hpp"
#include "../meta/type_transform.hpp"
#include "../exceptions/id_not_found.hpp"
#include "../exceptions/abstract_type.hpp"
#include "serializer/exceptions/create_type.hpp"
#include <stdexcept>
#include <type_traits>

/******************************************************************************/
/*                                 type table                                 */
/******************************************************************************/

/// @brief namespace serializer tools
namespace serializer::tools {

/// @brief Table that is used to get the identifiers of the serialized types.
/// @tparam Tyes Registers types.
template <typename... Types> struct TypeTable {
    using id_type = unsigned int; // TODO
    static constexpr size_t size = sizeof...(Types);
};

/* contains *******************************************************************/

/// @brief True if `T` is in `Table`
template <typename T, typename Table> struct has_type;

template <typename T, typename... Types>
struct has_type<T, TypeTable<Types...>> {
    static constexpr bool value = mtf::contains_v<T, Types...>;
};

/// @brief True if `T` is in `Table`
template <typename T, typename Table>
constexpr bool has_type_v = has_type<mtf::base_t<T>, Table>::value;

/* has id *********************************************************************/

/// @brief True if `id` is in `Table`
template <typename... Types>
constexpr bool hasId(size_t id, TypeTable<Types...>) {
    return id < TypeTable<Types...>::size;
}

/* get id *********************************************************************/

/// @brief Get the id of the Target type stored in the given table type.
/// @tparam Target Target type.
/// @tparam T Firt type in the table.
/// @tparam Ts Rest of the types in the table.
/// @parma _ Type table.
template <typename Target, typename T, typename... Ts>
constexpr inline TypeTable<T, Ts...>::id_type getId(TypeTable<T, Ts...>) {
    if constexpr (std::is_same_v<mtf::base_t<Target>, T>) {
        return 0;
    } else {
        static_assert(sizeof...(Ts) != 0, "error: type not found in id table.");
        return 1 + getId<Target>(TypeTable<Ts...>());
    }
}

/// @brief Get the id of a type from mem at pos.
/// @tparam T Type of the id.
/// @param mem Buffer containing the serialized data.
/// @param pos Start position in the buffer where the id is serialized.
template <typename TypeTable>
inline constexpr typename TypeTable::id_type getId(auto &mem, size_t pos = 0) {
    return *std::bit_cast<const typename TypeTable::id_type *>(mem.data() +
                                                               pos);
}

/* create *********************************************************************/

/// @brief Create a pointer of type T and put the result in elt. The pointer can
///        be shared, unique or a standard pointer.
///        Note: if the element is not a pointer the function does nothing.
/// @tparam T Type of the pointer
/// @param elt Element that will contain the result
/// @throw Error when T is an abstract class.
template <typename T> inline constexpr void create(auto &elt) {
    using Type = decltype(elt);
    if constexpr (!std::is_abstract_v<T>) {
        if constexpr (concepts::Pointer<Type> && requires(Type t) { t = new T(); }) {
            if (elt != nullptr) {
                delete elt;
            }
            elt = new T();
        } else if constexpr (mtf::is_shared_v<Type> && requires(Type t) { t = std::make_shared<T>(); }) {
            elt = std::make_shared<T>();
        } else if constexpr (mtf::is_unique_v<Type> && requires(Type t) { t = std::make_unique<T>(); }) {
            elt = std::make_unique<T>();
        } else if constexpr (requires(Type t) { t = T(); }) {
            elt = T();
        } else {
            throw exceptions::CreateTypeError<T>();
        }
    } else {
        throw exceptions::AbstractTypeError<T>();
    }
}

/// @brief Create a polymorphic type. The real type is found using the given id.
/// @tparam SuperType Type of the element.
/// @tparam T Firt type in the table.
/// @tparam Ts Rest of the types in the table.
/// @param id Identifier of the target type.
/// @parma _ Type table.
/// @parma elt Deserialize element, it will contains the result object.
template <typename SuperType, typename IdType, typename T, typename... Ts>
constexpr inline void createPolymorphic(IdType id, TypeTable<T, Ts...>,
                                        SuperType &elt) {
    if (id == 0) {
        create<T>(elt);
    } else {
        if constexpr (sizeof...(Ts)) {
            createPolymorphic(IdType(id - 1), TypeTable<Ts...>(), elt);
        }
    }
}

/// @brief Creates a element using the identifier.
/// @tparam TypeTable The type table.
/// @param id Identifier of the type to create.
/// @param elt Element that will contain the created type.
/// @throw Error when the id is not in the given type table.
template <typename TypeTable>
constexpr inline void createId(auto id, auto &elt) {
    if (!hasId(id, TypeTable())) [[unlikely]] {
        throw exceptions::IdNotFoundError(id);
    }
    if (id == getId<decltype(elt)>(TypeTable())) {
        create<mtf::base_t<decltype(elt)>>(elt);
    } else {
        createPolymorphic(id, TypeTable(), elt);
    }
}

/* apply id *******************************************************************/

/// @brief Apply a template lambda to the type with the identifier `id` in the
///        given type table.
/// @tparam T First type in the type table.
/// @tparam Ts Rest of the types in the type table.
/// @param id       Identifier of the target type.
/// @param _        Type table.
/// @param function Template lambda / functor to apply on the type. the
///                 operator() should be template parametrized with a type T
///                 that will correspond to the type of identifier id.
template <typename T, typename... Ts>
constexpr void applyId(auto id, TypeTable<T, Ts...>, auto function) {
    if (id == 0) {
        function.template operator()<T>();
    } else {
        if constexpr (sizeof...(Ts)) {
            applyId(typename TypeTable<T, Ts...>::id_type(id - 1),
                    TypeTable<Ts...>(), function);
        } else {
            throw std::logic_error("error: id not found");
        }
    }
}

} // end namespace serializer::tools

#endif
