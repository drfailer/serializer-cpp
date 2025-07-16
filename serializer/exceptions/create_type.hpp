#ifndef SERIALIZER_CREATE_TYPE_ERROR_HPP
#define SERIALIZER_CREATE_TYPE_ERROR_HPP
#include <exception>
#include <sstream>
#include <string>

/// @brief namespace serializer exception
namespace serializer::exceptions {

/// @brief Exception thrown when the type table cannot create a type.
/// @tparam T Non creatable type.
template <typename T>
class CreateTypeError : public std::exception {
  public:
    /// @brief Constructor
    CreateTypeError() {
        T *obj;
        std::ostringstream oss;
        oss << "error: connot create type '" << typeid(obj).name() << "'."
            << std::endl;
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
