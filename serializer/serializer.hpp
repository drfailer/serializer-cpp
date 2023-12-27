#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include <sstream>
#include "attrcontainer.hpp"
#include "convertor.hpp"

/******************************************************************************/
/*                                stringifier                                 */
/******************************************************************************/

/* stringifier ****************************************************************/
template <typename... Types>
class Serializer {
public:
    /* constructor & destructor ***********************************************/
    Serializer(Types &...args, std::string idsStr):
        container(args..., idsStr) {
            convertor = new Convertor();
        }
    ~Serializer() = default;

    /* serialize **************************************************************/
    std::string serialize(std::string className) const {
        std::ostringstream oss;
        oss << "{ __CLASS_NAME__: " << className << ", " << container.serialize(convertor) << " }";
        return oss.str();
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string& str) {
        container.deserialize(str, convertor);
    }

    /* set convertor **********************************************************/
    void setConvertor(Convertor *newConvertor) {
        delete convertor;
        convertor = newConvertor;
    }

private:
    AttrContainer<Types...> container;
    Convertor *convertor = nullptr;
};

#endif
