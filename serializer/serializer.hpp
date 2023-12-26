#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
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
    Serializer(Types &...args, std::string idsStr):
        container(args..., idsStr) { }
    ~Serializer() = default;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << "{ " << container.serialize() << " }";
        return oss.str();
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string& str) {
        container.deserialize(str);
    }

private:
    AttrContainer<Types...> container;
};

#endif
