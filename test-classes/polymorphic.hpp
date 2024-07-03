#ifndef POLYMORPHIC_HPP
#define POLYMORPHIC_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/serializer.hpp"
#include "serializer/tools/parser.hpp"
#include <vector>

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperClass {
    SERIALIZABLE_POLYMORPHIC(std::string, int);

  public:
    SuperClass(std::string _name = "", int _age = 0)
        : SERIALIZER(name, age), name(_name), age(_age) {}
    virtual ~SuperClass() {}

    virtual bool operator==(const SuperClass *other) const {
        return name == other->name && age == other->age;
    }

    /* accessors **************************************************************/
    void setAge(int age) { this->age = age; }
    void setName(std::string name) { this->name = name; }
    int getAge() const { return age; }
    std::string getName() const { return name; }

  private:
    std::string name;
    int age;
};

/******************************************************************************/
/*                              concrete class 1                              */
/******************************************************************************/

class Class1 : public SuperClass {
    SERIALIZABLE_SUPER(SuperClass, int, double);

  public:
    Class1(const std::string &name = "", int age = 0, int _x = 0, double _y = 0)
        : SuperClass(name, age), SERIALIZER(x, y), x(_x), y(_y) {}
    ~Class1() = default;

    /* accessors **************************************************************/
    void setX(int x) { this->x = x; }
    void setY(double y) { this->y = y; }
    int getX() const { return x; }
    double getY() const { return y; }

    /* operator== *************************************************************/
    bool operator==(const SuperClass *other) const override {
        if (const Class1 *c = dynamic_cast<const Class1 *>(other)) {
            return x == c->x && y == c->y && SuperClass::operator==(other);
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

class Class2 : public SuperClass {
    SERIALIZABLE_SUPER(SuperClass, std::string);

  public:
    Class2(const std::string &name = "", int age = 0,
           const std::string &_str = "")
        : SuperClass(name, age), SERIALIZER(str), str(_str) {}
    ~Class2() = default;

    /* accessors **************************************************************/
    void setStr(std::string str) { this->str = str; }
    std::string getStr() const { return str; }

    /* operator== *************************************************************/
    bool operator==(const SuperClass *other) const override {
        if (const Class2 *c = dynamic_cast<const Class2 *>(other)) {
            return str == c->str && SuperClass::operator==(other);
        }
        return false;
    }

  private:
    std::string str;
};

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct SuperConvertor : public serializer::Convertor<SuperClass*> {
    HANDLE_POLYMORPHIC(SuperClass, Class1, Class2);
};

/******************************************************************************/
/*                            abstract collection                             */
/******************************************************************************/

class SuperCollection {
    SERIALIZABLE_WITH_CONVERTOR(SuperConvertor, std::vector<SuperClass *>);

  public:
    SuperCollection() : SERIALIZER(elements) {}
    ~SuperCollection() {
        for (auto elt : elements) {
            delete elt;
        }
    }

    void push_back(SuperClass *element) { elements.push_back(element); }
    const std::vector<SuperClass *> &getElements() { return elements; }

  private:
    std::vector<SuperClass *> elements;
};

#endif
