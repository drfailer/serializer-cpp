#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include "attrcontainer.hpp"
#include "convertor.hpp"
#include <fstream>
#include <sstream>

// NOTE: we should use static assert to create useful error messages

#define SERIALIZER(...)                                                        \
    __serializer__(__VA_ARGS__, #__VA_ARGS__, typeid(*this).name())

// TODO: add the convertor in the template parameters
// TODO: add function with files names as input
#define SERIALIZABLE_WITH_CONVERTOR(Convertor, ...)                            \
  private:                                                                     \
    Serializer<Convertor, __VA_ARGS__> __serializer__;                         \
  public:                                                                      \
    std::string serialize() const { return __serializer__.serialize(); }       \
    void deserialize(std::string str) {                                        \
        return __serializer__.deserialize(str);                                \
    }                                                                          \
  private:

#define SERIALIZABLE(...) SERIALIZABLE_WITH_CONVERTOR(Convertor, __VA_ARGS__)

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

// TODO: add the convertor type as template parameter (may ad a test with is
// based of or a static assert in the class)
template <typename Conv, typename... Types> class Serializer {
  public:
    /* constructor & destructor ***********************************************/
    Serializer(Types &...args, std::string idsStr, std::string className)
        : container(args..., idsStr), className(className) {}
    ~Serializer() = default;

    /* serialize **************************************************************/
    std::string serialize() const {
        std::ostringstream oss;
        oss << "{ __CLASS_NAME__: " << className << ", "
            << container.serialize() << " }";
        return oss.str();
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string &str) {
        container.deserialize(str);
    }

  private:
    AttrContainer<Conv, Types...> container;
    std::string className;
};

#endif
