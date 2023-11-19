#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include <cstddef>
#include <tuple>
#include <vector>
#include <sstream>

#include <iostream>

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

// TODO: the toString function should be const => modify the Stringifiable and
// the Stringifier functions.

/******************************************************************************/
/*                                stringifier                                 */
/******************************************************************************/

/* stringifier ****************************************************************/
template <typename... Types>
class Stringifier {
public:
    /* constructor & destructor ***********************************************/
    Stringifier(Types &...args, std::string idsStr) : ptrs(args...) {
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
    ~Stringifier() = default;

    /* toString ***************************************************************/
    std::string toString() {
        oss.str(""); // security clear
        oss << "{ ";
        iter_on_ptrs(ptrs, std::make_index_sequence<sizeof...(Types)>());
        oss << " }";
        return oss.str();
    }

private:
    std::tuple<Types &...> ptrs;
    std::vector<std::string> ids;
    std::ostringstream oss;

    /* helper functions *******************************************************/
    template <typename H, typename... T>
    void toString_foreach_attr(H &head, T &...args) {
        int i = 1;
        oss << ids[0] << ": " << head;
        ([&]{
            oss << ", " << ids[i] << ": " << args;
            i++;
        }(), ...);
    }

    template <typename T, size_t... Is>
    void iter_on_ptrs(const T &t, std::index_sequence<Is...>) {
        toString_foreach_attr(std::get<Is>(t)...);
    }
};

#endif
