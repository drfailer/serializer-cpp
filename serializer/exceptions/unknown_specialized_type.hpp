#ifndef UKNOWN_SPECIALIZED_TYPE_HPP
#define UKNOWN_SPECIALIZED_TYPE_HPP
#include <exception>
#include <sstream>
#include <string_view>

namespace serializer::exceptions {

template <typename T> class UknownSpecializedTypeError : public std::exception {
  public:
    UknownSpecializedTypeError(std::string_view const &className) {
        T *obj;
        std::ostringstream oss;
        oss << "error: unknown type '" << className
            << "' that should inherit from '" << typeid(obj).name()
            << std::endl;
        oss << "Make sure that this type appears in the macro "
               "HANDLE_POLYMORPHIC.";
        msg = oss.str();
    }

    const char *what() const noexcept override { return msg.c_str(); }

  private:
    std::string msg;
};

} // namespace serializer::exceptions

#endif
