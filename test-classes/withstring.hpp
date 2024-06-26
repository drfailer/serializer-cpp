#ifndef WITHSTRING_HPP
#define WITHSTRING_HPP
#include "serializer/serializer.hpp"
#include <string>
#include <utility>

class WithString {
    SERIALIZABLE(int, std::string);
  public:
    WithString(int _x, std::string _str)
        : SERIALIZER(x, str), x(_x), str(std::move(_str)) {}
    ~WithString() = default;

    /* accessors **************************************************************/
    void setStr(std::string str) { this->str = str; }
    void setX(int x) { this->x = x; }
    std::string getStr() const { return str; }
    int getX() const { return x; }

  private:
    int x;
    std::string str = "";
};

#endif
