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
template <typename Conv, typename... Types> struct AttrContainer {
    std::string serialize() const {
        return "";
    }
    void deserialize(std::string) { }
};

/*
 * Specialisation for handeling more than one type.
 */
template <typename Conv, typename H, typename... Types> struct AttrContainer<Conv, H, Types...> {
    /* attributes *************************************************************/
    H &reference;
    std::string name;
    AttrContainer<Conv, Types...> next;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << Conv::serialize(reference);
        oss << ", " << next.serialize();
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string &str) {
        std::size_t idxName = str.find(name + ": "); // TODO: problem here -> add a function to the parser
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = Conv::template deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = Conv::template deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        }
        next.deserialize(str);
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
template <typename Conv, typename H> struct AttrContainer<Conv, H> {
    /* attributes *************************************************************/
    H &reference;
    std::string name;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << name << ": " << Conv::serialize(reference);
        return oss.str();
    }

    /* deserialize ************************************************************/
    void deserialize(const std::string &str) {
        std::size_t idxName = str.find(name + ": "); // TODO: problem here -> add a function to the parser
        std::size_t idxValue = idxName + name.size() + 2;
        std::size_t idxEnd = findEndValueIndex(str, idxValue);

        if constexpr (std::is_pointer_v<H>) {
            if (reference != nullptr) {
                delete reference;
            }
            reference = Conv::template deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        } else {
            reference = Conv::template deserialize<H>(
                str.substr(idxValue, idxEnd - idxValue));
        }

        reference = Conv::template deserialize<decltype(reference)>(
            str.substr(idxValue, idxEnd - idxValue));
    }

    /* constructor ************************************************************/
    AttrContainer(H &head, const std::string &idsStr)
        : reference(head), name(idsStr) {}
};

#endif
