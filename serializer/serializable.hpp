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

// TODO: remove this or make it abstract and non template
template <typename... Types> class Serializable {
  public:
    /* constructor & destructor ***********************************************/
    Serializable(Types &...vars, std::string varsStr, std::string className)
        : _serializer(vars..., varsStr, className) {}
    Serializable() { }
    virtual ~Serializable() {}

    /* serialize **************************************************************/
    virtual std::string serialize() const { return _serializer.serialize(); }

    /* deserialize  ***********************************************************/
    virtual void deserialize(const std::string &str) { _serializer.deserialize(str); }

  private:
    Serializer<Types...> _serializer;
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
