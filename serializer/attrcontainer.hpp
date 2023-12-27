#ifndef ATTRCONTAINER_HPP
#define ATTRCONTAINER_HPP
#include "convertor.hpp"
#include "parser.hpp"
#include <string>
#include <type_traits>

/******************************************************************************/
/*                            attribute container                             */
/******************************************************************************/

/*
 * Data structure that acts like a map with multiple types. It is used to store
 * the identifiers and the references on the attributes of the serialized class.
 */
template <typename... Types> struct AttrContainer {};

/*
 * Specialisation for handeling more than one type.
 */
template <typename H, typename... Types> struct AttrContainer<H, Types...> {
    /* attributes *************************************************************/
    H &reference;
    std::string name;
    AttrContainer<Types...> next;

    /* serialize **************************************************************/
    std::string serialize(Convertor *convertor) const {
        std::ostringstream oss;
        oss << name << ": " << convertor->serialize(reference);
        oss << ", " << next.serialize(convertor);
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string &str, Convertor *convertor) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = convertor->deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = convertor->deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        }
        next.deserialize(str, convertor);
    }

    /* constructor ************************************************************/
    AttrContainer(H &head, Types &...types, const std::string &idsStr)
        : reference(head), name(idsStr.substr(0, idsStr.find(","))),
          next(types..., idsStr.substr(nextId(idsStr))) {}
};

/*
 * Specialisation with one type. This specialisation doesn't have the next
 * attribute.
 */
template <typename H> struct AttrContainer<H> {
    /* attributes *************************************************************/
    H &reference;
    std::string name;

    /* serialize **************************************************************/
    std::string serialize(Convertor *convertor) const {
        std::ostringstream oss;
        oss << name << ": " << convertor->serialize(reference);
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string &str, Convertor *convertor) {
        std::size_t idxName = str.find(name);
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = convertor->deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = convertor->deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        }

        reference = convertor->deserialize<decltype(reference)>(
            str.substr(idxValue, idxEnd - idxValue));
    }

    /* constructor ************************************************************/
    AttrContainer(H &head, const std::string &idsStr)
        : reference(head), name(idsStr) {}
};

#endif
