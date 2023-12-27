#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP
#include "convertor.hpp"
#include "serializer.hpp"
#include <sstream>
#include <typeinfo>

/******************************************************************************/
/*                               stringifiable                                */
/******************************************************************************/

// macro for calling the constructor
#define serializable(...) Serializable(__VA_ARGS__, #__VA_ARGS__, typeid(*this).name())

template <typename... Types> class Serializable {
  public:
    /* constructor & destructor ***********************************************/
    Serializable(Types &...vars, std::string varsStr, std::string className)
        : serializer(vars..., varsStr), className(className) {}
    virtual ~Serializable() {}

    /* serialize **************************************************************/
    std::string serialize() const { return serializer.serialize(className); }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string &str) { serializer.deserialize(str); }

    /* set convertor **********************************************************/
    void setConvertor(Convertor *convertor) {
        serializer.setConvertor(convertor);
    }

  private:
    Serializer<Types...> serializer;
    std::string className;
};

/******************************************************************************/
/*                                 functions                                  */
/******************************************************************************/

template <typename... Types>
inline std::ostream &operator<<(std::ostream &os,
                                const Serializable<Types...> &s) {
    os << s.serialize();
    return os;
}

#endif
