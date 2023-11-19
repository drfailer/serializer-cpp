#ifndef STRINGIFIABLE_HPP
#define STRINGIFIABLE_HPP
#include <sstream>
#include "stringifier.hpp"

/******************************************************************************/
/*                               stringifiable                                */
/******************************************************************************/

#define stringifiable(...) Stringifiable(__VA_ARGS__, #__VA_ARGS__)

template<typename ...Types>
class Stringifiable {
public:
    std::string toString() {
        return strf.toString();
    }
    Stringifiable(Types& ...vars, std::string varsStr): strf(vars..., varsStr) { }
    virtual ~Stringifiable() {}

private:
Stringifier<Types...> strf;
};

template<typename ...Types>
inline std::ostream& operator<<(std::ostream& os, Stringifiable<Types...>& s) {
    os << s.toString();
    return os;
}

#endif
