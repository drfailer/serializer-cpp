#ifndef STRINGIFIABLE_HPP
#define STRINGIFIABLE_HPP
#include <sstream>

/******************************************************************************/
/*                               stringifiable                                */
/******************************************************************************/

class Stringifiable {
    public:
        virtual std::string toString() = 0;
        virtual ~Stringifiable() {}
};

inline std::ostream& operator<<(std::ostream& os, Stringifiable& s) {
    os << s.toString();
    return os;
}

#endif
