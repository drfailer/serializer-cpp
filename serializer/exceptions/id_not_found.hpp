#ifndef SERIALIZER_UNSUPPORTED_ID_NOT_FOUND_ERROR_HPP
#define SERIALIZER_UNSUPPORTED_ID_NOT_FOUND_ERROR_HPP
#include <exception>
#include <sstream>
#include <string>

/// @brief namespace serializer exception
namespace serializer::exceptions {

/// @brief Exception for id not found in the type table.
class IdNotFoundError : public std::exception {
  public:
    /// @brief Constructor
    IdNotFoundError(auto id) {
        std::ostringstream oss;
        oss << "error: the identifier '" << id
            << "' was not found in the given type table.";
        msg = oss.str();
    }

    /// @brief what
    const char *what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg; ///< message for what.
};

} // namespace serializer::exceptions

#endif
