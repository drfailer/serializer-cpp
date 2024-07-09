#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP
#include "serializer.hpp"
#include "serializer/convertor/convertor.hpp"

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

/// @brief Inits the serializer attribute in the serialized class constructor.
/// @param ... Attributes to serialize (must be valid references).
#define SERIALIZER(...)                                                        \
    __serializer__(__VA_ARGS__, #__VA_ARGS__, typeid(*this).name())

/// @brief Used to create a serializable class with the default convertor.
#define DEFAULT_CONVERTOR serializer::Convertor<>

/// @brief Generates de the default implementation for serialization /
///        deserialization functions.
#define DEFAULT_FN_IMPL                                                        \
    std::string serialize() const { return __serializer__.serialize(); }       \
    std::string serialize(std::string &str) const {                            \
        return __serializer__.serialize(str);                                  \
    }                                                                          \
    void serializeFile(const std::string &fn) const {                          \
        return __serializer__.serializeFile(fn);                               \
    }                                                                          \
    void deserialize(const std::string &str) {                                 \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    void deserialize(std::string_view &str) {                                  \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    void deserializeFile(const std::string &fn) {                              \
        __serializer__.deserializeFile(fn);                                    \
    }

/// @brief Generates implementation for polymorphic classes with virtual
///        methods. (note: these classes are not deserializable by default)
#define POLYMORPHIC_FN_IMPL                                                    \
    virtual std::string serialize() const {                                    \
        return __serializer__.serialize();                                     \
    }                                                                          \
    virtual std::string serialize(std::string &str) const {                    \
        return __serializer__.serialize(str);                                  \
    }                                                                          \
    virtual void serializeFile(const std::string &fn) const {                  \
        return __serializer__.serializeFile(fn);                               \
    }                                                                          \
    virtual void deserialize(const std::string &str) {                         \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    virtual void deserialize(std::string_view &str) {                          \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    virtual void deserializeFile(const std::string &fn) {                      \
        __serializer__.deserializeFile(fn);                                    \
    }

/// @brief Generates de the implementation for serialization / deserialization
///        functions where the super class has to be serialized too.
#define SUPER_FN_IMPL(Super)                                                   \
    std::string serialize() const override {                                   \
        return __serializer__.serialize() + Super::serialize();                \
    }                                                                          \
    std::string serialize(std::string &str) const override {                   \
        return Super::serialize(__serializer__.serialize(str));                \
    }                                                                          \
    void serializeFile(const std::string &fn) const override {                 \
        std::ofstream file(fn);                                                \
        file << serialize() << std::endl;                                      \
    }                                                                          \
    void deserialize(const std::string &str) override {                        \
        std::string_view strv = str;                                           \
        deserialize(strv);                                                     \
    }                                                                          \
    void deserialize(std::string_view &str) override {                         \
        __serializer__.deserialize(str);                                       \
        Super::deserialize(str);                                               \
    }                                                                          \
    void deserializeFile(const std::string &fn) override {                     \
        std::ifstream file(fn);                                                \
        std::ostringstream oss;                                                \
        oss << file.rdbuf();                                                   \
        deserialize(oss.str());                                                \
    }

/// @brief Generates the code in the serialized class. It adds a serializer
///        attribute and serialization / deserialization functions (the
///        implementation is configurable)
#define __SERIALIZABLE__(CONV, IMPL, ...)                                      \
  private:                                                                     \
    serializer::Serializer<CONV, __VA_ARGS__> __serializer__;                  \
                                                                               \
  public:                                                                      \
    IMPL;                                                                      \
                                                                               \
  private:

/// @brief Generates an empty serializer (we may want an abstract class without
///        any attribute to be serializable).
#define SERIALIZABLE_EMPTY()                                                   \
  private:                                                                     \
    serializer::Serializer<DEFAULT_CONVERTOR> __serializer__;                  \
                                                                               \
  public:                                                                      \
    POLYMORPHIC_FN_IMPL;                                                       \
                                                                               \
  private:

/******************************************************************************/
/*                                 shorthands                                 */
/******************************************************************************/

/// @brief Generates the code for the default serializable class.
#define SERIALIZABLE(...)                                                      \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, DEFAULT_FN_IMPL, __VA_ARGS__)

/// @brief Generates the code for the default serializable polymorphic class.
#define SERIALIZABLE_POLYMORPHIC(...)                                          \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, POLYMORPHIC_FN_IMPL, __VA_ARGS__)

/// @brief Generates the code for the serializable with a custom convertor.
#define SERIALIZABLE_WITH_CONVERTOR(Convertor, ...)                            \
    __SERIALIZABLE__(Convertor, DEFAULT_FN_IMPL, __VA_ARGS__)

/// @brief Generates the code for the serializable with a custom implementation.
#define SERIALIZABLE_WITH_IMPL(IMPL, ...)                                      \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, IMPL, __VA_ARGS__)

/// @brief Generates the code for the serializable with the default convertor
///        and use the implementation for serializing the super class.
#define SERIALIZABLE_SUPER(Super, ...)                                         \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, SUPER_FN_IMPL(Super), __VA_ARGS__)

/// @brief Generates the code for the serializable with a custom convertor and
///        use the implementation for serializing the super class.
#define SERIALIZABLE_CUSTOM(Convertor, Super, ...)                             \
    __SERIALIZABLE__(Convertor, SUPER_FN_IMPL(Super), __VA_ARGS__)

#endif
