#ifndef WITHSTRING_HPP
#define WITHSTRING_HPP
#include "serializer/serializable.hpp"
#include <string>

class WithString : public Serializable<int, std::string> {
  public:
    WithString(int _x, std::string _str)
        : serializable(x, str), x(_x), str(_str) {}
    ~WithString() = default;

    /* accessors **************************************************************/
    void setStr(std::string str) { this->str = str; }
    void setX(int x) { this->x = x; }
    std::string getStr() const { return str; }
    int getX() const { return x; }

  private:
    int x;
    std::string str;
};

#endif
