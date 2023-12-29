#ifndef STRINGIFIER_HPP
#define STRINGIFIER_HPP
#include "attrcontainer.hpp"
#include "convertor.hpp"
#include <fstream>
#include <sstream>

// NOTE: we should use static asserts to create useful error messages
// NOTE: removing the ' ' (spaces) in the serialized strings could be
// interesting

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

#define SERIALIZER(...)                                                        \
    __serializer__(__VA_ARGS__, #__VA_ARGS__, typeid(*this).name())

#define SERIALIZABLE_WITH_CONVERTOR(Convertor, ...)                            \
  private:                                                                     \
    Serializer<Convertor, __VA_ARGS__> __serializer__;                         \
                                                                               \
  public:                                                                      \
    std::string serialize() const { return __serializer__.serialize(); }       \
    void serializeFile(const std::string &fn) const {                          \
        return __serializer__.serializeFile(fn);                               \
    }                                                                          \
    void deserialize(const std::string &str) {                                 \
        return __serializer__.deserialize(str);                                \
    }                                                                          \
    void deserializeFile(const std::string &fn) {                              \
        return __serializer__.deserializeFile(fn);                             \
    }                                                                          \
                                                                               \
  private:

#define SERIALIZABLE(...) SERIALIZABLE_WITH_CONVERTOR(Convertor, __VA_ARGS__)

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

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

    void serializeFile(const std::string &fileName) const {
        std::ofstream file(fileName);
        file << serialize() << std::endl;
    }

    /* deserialize  ***********************************************************/
    void deserialize(const std::string &str) { container.deserialize(str); }

    void deserializeFile(const std::string &fileName) {
        std::ifstream file(fileName);
        std::ostringstream oss;
        oss << file.rdbuf();
        deserialize(oss.str()); // NOTE: it would be nice to be able to read the
                                // buffer directly (may be faster than strings)
    }

  private:
    AttrContainer<Conv, Types...> container;
    std::string className;
};

#endif
