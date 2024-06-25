#ifndef ATTRCONTAINER_HPP
#define ATTRCONTAINER_HPP
#include "convertor.hpp"
#include "parser.hpp"
#include <map>
#include <string>
#include <type_traits>

namespace serializer {

/******************************************************************************/
/*                            attribute container                             */
/******************************************************************************/

/*
 * Data structure that acts like a map with multiple types. It is used to store
 * the identifiers and the references on the attributes of the serialized class.
 */
template <typename Conv, typename... Types> struct AttrContainer {
    std::string serialize(std::string &str) const { return str; }
    void deserialize(const std::map<std::string, std::string> &) {}

    AttrContainer() {}
    AttrContainer(const std::string &) {}
};

/*
 * Specialisation for handeling more than one type.
 */
template <typename Conv, typename H, typename... Types>
struct AttrContainer<Conv, H, Types...> {
    /* attributes *************************************************************/
    H &reference;
    std::string name;
    AttrContainer<Conv, Types...> next;

    /* serialize **************************************************************/
    std::string serialize(std::string &str) const {
        Conv::serialize(reference, str);
        if constexpr (sizeof...(Types) > 0) {
            next.serialize(str);
        }
    }

    /* deserialize ************************************************************/
    void deserialize(const std::map<std::string, std::string> &elts) {
        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
                reference = nullptr;
            }
        }
        reference = Conv::template deserialize<H>(elts.at(name));
        next.deserialize(elts);
    }

    /* constructor ************************************************************/
    AttrContainer(H &head, Types &...types, const std::string &idsStr)
        : reference(head), name(idsStr.substr(0, idsStr.find(","))),
          next(types..., idsStr.substr(parser::nextId(idsStr))) {}
};

} // namespace serializer

#endif
