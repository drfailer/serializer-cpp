#ifndef POLYMORPHIC_HPP
#define POLYMORPHIC_HPP
#include "serializer/convertor.hpp"
#include "serializer/parser.hpp"
#include "serializer/serializer.hpp"
#include "test-classes/simple.hpp"
#include <iostream>
#include <type_traits>
#include <vector>

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperAbstract {
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
    SERIALIZABLE(int, double);

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
    SERIALIZABLE(std::string);

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

struct AbstractCollectionConvertor {
    CONVERTOR;

    /* deserialize function for SuperAbstract* type */
    deserialize_custom_type(SuperAbstract *, const std::string &str) {
        std::string className = getClassName(str);
        SuperAbstract *out = nullptr;

        // we use class_name to find out the concrete type of element
        if (className == class_name(Concrete1)) {
            Concrete1 *c1 = new Concrete1();
            c1->deserialize(str);
            out = c1;
        } else if (className == class_name(Concrete2)) {
            Concrete2 *c2 = new Concrete2();
            c2->deserialize(str);
            out = c2;
        }
        return out;
    }

    static std::string serialize(SuperAbstract *elt) {
        if (Concrete1 *c1 = dynamic_cast<Concrete1 *>(elt)) {
            return c1->serialize();
        } else if (Concrete2 *c2 = dynamic_cast<Concrete2 *>(elt)) {
            return c2->serialize();
        }
        return "nullptr";
    }
};

/******************************************************************************/
/*                            abstract collection                             */
/******************************************************************************/

class AbstractCollection {
    SERIALIZABLE_WITH_CONVERTOR(AbstractCollectionConvertor,
                                std::vector<SuperAbstract *>);

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
