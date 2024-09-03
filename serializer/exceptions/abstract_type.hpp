#ifndef SERIALIZER_UNSUPPORTED_ABSTRACT_TYPE_ERROR_HPP
#define SERIALIZER_UNSUPPORTED_ABSTRACT_TYPE_ERROR_HPP
#include <exception>
#include <sstream>
#include <string>

/// @brief namespace serializer exception
namespace serializer::exceptions {

/// @brief Exception for unsupported types.
/// @tparam T Unsupported type.
template <typename T> class AbstractTypeError : public std::exception {
  public:
    /// @brief Constructor
    AbstractTypeError() {
        T *obj;
        std::ostringstream oss;
        oss << "error: the identifier of the abstract type '"
            << typeid(obj).name()
            << "' as been found in the type table.";
        msg = oss.str();
    }

    /// @brief what
    const char *what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg; ///< message for what.
};

} // namespace serializer::exceptions

#endif
