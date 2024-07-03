#ifndef ABSTRACT_HPP
#define ABSTRACT_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/serializer.hpp"
#include <iostream>
#include <vector>

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperAbstract {
    SERIALIZABLE_EMPTY();
  public:
    SuperAbstract() {}
    virtual ~SuperAbstract() {}

    virtual void method() = 0;
    virtual bool operator==(const SuperAbstract *) const = 0;
};

/******************************************************************************/
/*                              concrete class 1                              */
/******************************************************************************/

class Concrete1 : public SuperAbstract {
    SERIALIZABLE_SUPER(SuperAbstract, int, double);

  public:
    Concrete1(int _x = 0, double _y = 0) : SERIALIZER(x, y), x(_x), y(_y) {}
    ~Concrete1() = default;

    /* accessors **************************************************************/
    void setX(int x) { this->x = x; }
    void setY(double y) { this->y = y; }
    int getX() const { return x; }
    double getY() const { return y; }

    /* method *****************************************************************/
    void method() override { std::cout << "Concrete1" << std::endl; }

    /* operator== *************************************************************/
    bool operator==(const SuperAbstract *other) const override {
        if (const Concrete1 *c = dynamic_cast<const Concrete1 *>(other)) {
            return x == c->x && y == c->y;
        }
        return false;
    }

  private:
    int x;
    double y;
};

/******************************************************************************/
/*                              concrete class 2                              */
/******************************************************************************/

class Concrete2 : public SuperAbstract {
    SERIALIZABLE_SUPER(SuperAbstract, std::string);

  public:
    Concrete2(const std::string &_str = "") : SERIALIZER(str), str(_str) {}
    ~Concrete2() = default;

    /* accessors **************************************************************/
    void setStr(std::string str) { this->str = str; }
    std::string getStr() const { return str; }

    /* method *****************************************************************/
    void method() override { std::cout << "Concrete2" << std::endl; }

    /* operator== *************************************************************/
    bool operator==(const SuperAbstract *other) const override {
        if (const Concrete2 *c = dynamic_cast<const Concrete2 *>(other)) {
            return str == c->str;
        }
        return false;
    }

  private:
    std::string str;
};

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

/* we use a custom convertor for handling generics */
struct Test : public serializer::Convertor<SuperAbstract*> {
    HANDLE_POLYMORPHIC(SuperAbstract, Concrete1, Concrete2)
};

/******************************************************************************/
/*                            abstract collection                             */
/******************************************************************************/

class AbstractCollection {
    SERIALIZABLE_WITH_CONVERTOR(Test, std::vector<SuperAbstract *>);

  public:
    AbstractCollection() : SERIALIZER(elements) {}
    ~AbstractCollection() {
        for (auto elt : elements) {
            delete elt;
        }
    }

    /* accessors **************************************************************/
    void push_back(SuperAbstract *element) { elements.push_back(element); }
    const std::vector<SuperAbstract *> &getElements() { return elements; }

  private:
    std::vector<SuperAbstract *> elements;
};

#endif
