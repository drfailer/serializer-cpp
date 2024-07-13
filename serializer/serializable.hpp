#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP
#include "serializer.hpp"
#include "serializer/convertor/convertor.hpp"
#include "tools/dynamic_array.hpp"

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

/// @brief Inits the serializer attribute in the serialized class constructor.
/// @param ... Attributes to serialize (must be valid references).
#define SERIALIZER(...) __serializer__(__VA_ARGS__, typeid(*this).name())

/// @brief Used to create a serializable class with the default convertor.
#define DEFAULT_CONVERTOR serializer::Convertor<>

/// @brief Generates de the default implementation for serialization /
///        deserialization functions.
#define DEFAULT_FN_IMPL                                                        \
    std::string serialize() const { return __serializer__.serialize(); }       \
    std::string &serialize(std::string &__str__) const {                       \
        return __serializer__.serialize(__str__);                              \
    }                                                                          \
    void serializeFile(const std::string &__fn__) const {                      \
        return __serializer__.serializeFile(__fn__);                           \
    }                                                                          \
    void deserialize(const std::string &__str__) {                             \
        __serializer__.deserialize(__str__);                                   \
    }                                                                          \
    void deserialize(std::string_view &__str__) {                              \
        __serializer__.deserialize(__str__);                                   \
    }                                                                          \
    void deserializeFile(const std::string &__fn__) {                          \
        __serializer__.deserializeFile(__fn__);                                \
    }

/// @brief Generates implementation for polymorphic classes with virtual
///        methods. (note: these classes are not deserializable by default)
#define POLYMORPHIC_FN_IMPL                                                    \
    virtual std::string serialize() const {                                    \
        return __serializer__.serialize();                                     \
    }                                                                          \
    virtual std::string &serialize(std::string &__str__) const {               \
        return __serializer__.serialize(__str__);                              \
    }                                                                          \
    virtual void serializeFile(const std::string &__fn__) const {              \
        return __serializer__.serializeFile(__fn__);                           \
    }                                                                          \
    virtual void deserialize(const std::string &__str__) {                     \
        __serializer__.deserialize(__str__);                                   \
    }                                                                          \
    virtual void deserialize(std::string_view &__str__) {                      \
        __serializer__.deserialize(__str__);                                   \
    }                                                                          \
    virtual void deserializeFile(const std::string &__fn__) {                  \
        __serializer__.deserializeFile(__fn__);                                \
    }

/// @brief Generates de the implementation for serialization / deserialization
///        functions where the super class has to be serialized too.
#define SUPER_FN_IMPL(Super)                                                   \
    std::string serialize() const override {                                   \
        return __serializer__.serialize() + Super::serialize();                \
    }                                                                          \
    std::string &serialize(std::string &__str__) const override {              \
        return Super::serialize(__serializer__.serialize(__str__));            \
    }                                                                          \
    void serializeFile(const std::string &__fn__) const override {             \
        std::ofstream file(__fn__);                                            \
        file << serialize() << std::endl;                                      \
    }                                                                          \
    void deserialize(const std::string &__str__) override {                    \
        std::string_view strv = __str__;                                       \
        deserialize(strv);                                                     \
    }                                                                          \
    void deserialize(std::string_view &__str__) override {                     \
        __serializer__.deserialize(__str__);                                   \
        Super::deserialize(__str__);                                           \
    }                                                                          \
    void deserializeFile(const std::string &__fn__) override {                 \
        std::ifstream file(__fn__);                                            \
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
