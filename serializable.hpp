#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP
#include <sstream>
#include "serializer.hpp"

/******************************************************************************/
/*                               stringifiable                                */
/******************************************************************************/

// macro for calling the constructor
#define serializable(...) Serializable(__VA_ARGS__, #__VA_ARGS__)

template<typename ...Types>
class Serializable {
public:
    std::string serialize() const {
        return strf.serialize();
    }

    void deserialize(const std::string& str) {
        strf.deserialize(str);
    }

    Serializable(Types& ...vars, std::string varsStr): strf(vars..., varsStr) { }
    virtual ~Serializable() {}

private:
Serializer<Types...> strf;
};

template<typename ...Types>
inline std::ostream& operator<<(std::ostream& os, const Serializable<Types...>& s) {
    os << s.serialize();
    return os;
}

#endif
