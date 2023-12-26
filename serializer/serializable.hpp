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
    /* constructor & destructor ***********************************************/
    Serializable(Types& ...vars, std::string varsStr):
        serializer(vars..., varsStr) { }
    virtual ~Serializable() { }

    /* serialize **************************************************************/
    std::string serialize() const {
        return serializer.serialize();
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string& str) {
        serializer.deserialize(str);
    }

private:
    Serializer<Types...> serializer;
};

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

template<typename ...Types>
inline std::ostream& operator<<(std::ostream& os, const Serializable<Types...>& s) {
    os << s.serialize();
    return os;
}

#endif
