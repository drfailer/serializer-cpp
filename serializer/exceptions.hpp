#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <exception>
#include <sstream>
#include <string>

namespace serializer::exceptions {

template <typename T>
class UnsupportedTypeError : public std::exception {
  public:
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

    const char* what() const noexcept override {
        return msg.c_str();
    }

  private:
    std::string msg;
};

}

#endif
