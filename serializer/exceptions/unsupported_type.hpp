#ifndef SERIALIZER_UNSUPPORTED_TYPE_ERROR_HPP
#define SERIALIZER_UNSUPPORTED_TYPE_ERROR_HPP
#include <exception>
#include <sstream>
#include <string>

/// @brief namespace serializer exception
namespace serializer::exceptions {

/// @brief Exception for unsupported types.
/// @tparam T Unsupported type.
template <typename T>
class UnsupportedTypeError : public std::exception {
  public:
    /// @brief Constructor
    UnsupportedTypeError() {
        T *obj;
        std::ostringstream oss;
        oss << "error: the default convertor doesn't support the type '"
            << typeid(obj).name() << "'." << std::endl;
        oss << "Consider making this type serializable or create a custom "
               "convertor implementation to add support for this type "
               "manually.";
        msg = oss.str();
    }

    /// @brief what
    const char* what() const noexcept override {
        return msg.c_str();
    }

  private:
    std::string msg; ///< message for what.
};

} // end namespace serializer::exception

#endif
