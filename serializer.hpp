#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include <cstddef>
#include <tuple>
#include <vector>
#include <sstream>
#include "attrcontainer.hpp"

/******************************************************************************/
/*                                stringifier                                 */
/******************************************************************************/

/* stringifier ****************************************************************/
template <typename... Types>
class Serializer {
public:
    /* constructor & destructor ***********************************************/
    Serializer(Types &...args, std::string idsStr) : ptrs(args...) {
        if (ids.size() == 0) { // security for copy constructors
            // parse ids
            size_t start = 0;
            while (start < idsStr.size()) {
                size_t end = idsStr.find(",", start);
                ids.push_back(idsStr.substr(start, end - start));
                while (end < idsStr.size() &&
                        (idsStr[end] == ' ' || idsStr[end] == ','))
                    ++end; // skip spaces and comas
                start = end;
            }
        }
    }
    ~Serializer() = default;

    /* toString ***************************************************************/
    std::string toString() const {
        std::ostringstream oss;
        oss << "{ "
            << iter_on_ptrs(ptrs, std::make_index_sequence<sizeof...(Types)>())
            << " }";
        return oss.str();
    }

private:
    std::tuple<Types &...> ptrs;
    std::vector<std::string> ids;

    /* helper functions *******************************************************/
    template <typename H, typename... T>
    std::string toString_foreach_attr(H &head, T &...args) const {
        std::ostringstream oss;
        int i = 1;
        oss << ids[0] << ": " << head;
        ([&]{
            oss << ", " << ids[i] << ": " << args;
            i++;
        }(), ...);
        return oss.str();
    }

    template <typename T, size_t... Is>
    std::string iter_on_ptrs(const T &t, std::index_sequence<Is...>) const {
        return toString_foreach_attr(std::get<Is>(t)...);
    }
};

#endif
