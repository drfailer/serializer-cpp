#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

/* Inits the serializer attribute in the serialized class constructor */
#define SERIALIZER(...)                                                        \
    __serializer__(__VA_ARGS__, #__VA_ARGS__, typeid(*this).name())

/* Used to create a serializable class with the default convertor. */
#define DEFAULT_CONVERTOR serializer::Convertor

/*
 * Generates de the default implementation for serialization / deserialization
 * functions.
 */
#define DEFAULT_FN_IMPL                                                        \
    std::string serialize() const { return __serializer__.serialize(); }       \
    void serializeFile(const std::string &fn) const {                          \
        return __serializer__.serializeFile(fn);                               \
    }                                                                          \
    void deserialize(const std::string &str) {                                 \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    void deserializeFile(const std::string &fn) {                              \
        __serializer__.deserializeFile(fn);                                    \
    }

/*
 * Generates implementation for polymophic classes with virtual methods.
 * (note: this classes are not deserializable by default)
 */
#define POLYMORPHIC_FN_IMPL                                                    \
    virtual std::string serialize() const {                                    \
        return __serializer__.serialize();                                     \
    }                                                                          \
    virtual void serializeFile(const std::string &fn) const {                  \
        return __serializer__.serializeFile(fn);                               \
    }                                                                          \
    virtual void deserialize(const std::string &str) {                         \
        __serializer__.deserialize(str);                                       \
    }                                                                          \
    virtual void deserializeFile(const std::string &fn) {                      \
        __serializer__.deserializeFile(fn);                                    \
    }

/*
 * Generates de the implementation for serialization / deserialization functions
 * where the super class has to be serialized too.
 */
#define SUPER_FN_IMPL(Super)                                                   \
    std::string serialize() const override {                                   \
        std::ostringstream oss;                                                \
        oss << "{ __THIS__: " << __serializer__.serialize() << ", "            \
            << "__SUPER__: " << Super::serialize() << " }";                    \
        return oss.str();                                                      \
    }                                                                          \
    void serializeFile(const std::string &fn) const override {                 \
        std::ofstream file(fn);                                                \
        file << serialize() << std::endl;                                      \
    }                                                                          \
    void deserialize(const std::string &str) override {                        \
        Super::deserialize(serializer::parser::getSuperValue(str));            \
        __serializer__.deserialize(serializer::parser::getThisValue(str));     \
    }                                                                          \
    void deserializeFile(const std::string &fn) override {                     \
        std::ifstream file(fn);                                                \
        std::ostringstream oss;                                                \
        oss << file.rdbuf();                                                   \
        deserialize(oss.str());                                                \
    }

/*
 * Generates the code in the serialized class. It adds a serializer attribute
 * and serialization / deserialization functions (the implementation is
 * configurable);
 */
#define __SERIALIZABLE__(CONV, IMPL, ...)                                      \
  private:                                                                     \
    serializer::Serializer<CONV, __VA_ARGS__> __serializer__;                  \
                                                                               \
  public:                                                                      \
    IMPL;                                                                      \
                                                                               \
  private:

/*
 * Generates an empty serilizer (we may wan't an abstract class without any
 * attribute to be serilizable).
 */
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

/* Generates the code for the default serializable class. */
#define SERIALIZABLE(...)                                                      \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, DEFAULT_FN_IMPL, __VA_ARGS__)

/* Generates the code for the default serializable polymophic class. */
#define SERIALIZABLE_POLYMORPHIC(...)                                          \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, POLYMORPHIC_FN_IMPL, __VA_ARGS__)

/* Generates the code for the serializable with a custom convertor. */
#define SERIALIZABLE_WITH_CONVERTOR(Convertor, ...)                            \
    __SERIALIZABLE__(Convertor, DEFAULT_FN_IMPL, __VA_ARGS__)

/* Generates the code for the serializable with a custom implementation. */
#define SERIALIZABLE_WITH_IMPL(IMPL, ...)                                      \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, IMPL, __VA_ARGS__)

/*
 * Generates the code for the serializable with the default convertor and use
 * the implementation for serializing the super class.
 */
#define SERIALIZABLE_SUPER(Super, ...)                                         \
    __SERIALIZABLE__(DEFAULT_CONVERTOR, SUPER_FN_IMPL(Super), __VA_ARGS__)

/*
 * Generates the code for the serializable with a custom convertor and use
 * the implementation for serializing the super class.
 */
#define SERIALIZABLE_CUSTOM(Convertor, Super, ...)                             \
    __SERIALIZABLE__(Convertor, SUPER_FN_IMPL(Super), __VA_ARGS__)

#endif
