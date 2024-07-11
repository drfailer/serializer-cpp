#ifndef POLYMORPHIC_HPP
#define POLYMORPHIC_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/serializer.hpp"
#include <vector>

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperClass {
    SERIALIZABLE_POLYMORPHIC(std::string, int);

  public:
    explicit SuperClass(std::string _name = "", int _age = 0)
        : SERIALIZER(name_, age_), name_(std::move(_name)), age_(_age) {}
    virtual ~SuperClass() = default;

    virtual bool operator==(const SuperClass *other) const {
        return name_ == other->name_ && age_ == other->age_;
    }

    /* accessors **************************************************************/
    void age(int age) { this->age_ = age; }
    void name(std::string name) { this->name_ = std::move(name); }
    [[nodiscard]] int age() const { return age_; }
    [[nodiscard]] std::string const &name() const { return name_; }

  private:
    std::string name_;
    int age_;
};

/******************************************************************************/
/*                              concrete class 1                              */
/******************************************************************************/

class Class1 : public SuperClass {
    SERIALIZABLE_SUPER(SuperClass, int, double);

  public:
    explicit Class1(std::string const &name = "", int age = 0, int x = 0, double y = 0)
        : SuperClass(name, age), SERIALIZER(x_, y_), x_(x), y_(y) {}

    /* accessors **************************************************************/
    void x(int x) { this->x_ = x; }
    void y(double y) { this->y_ = y; }
    [[nodiscard]] int x() const { return x_; }
    [[nodiscard]] double y() const { return y_; }

    /* operator== *************************************************************/
    bool operator==(const SuperClass *other) const override {
        if (const auto *c = dynamic_cast<const Class1 *>(other)) {
            return x_ == c->x_ && y_ == c->y_ && SuperClass::operator==(other);
        }
        return false;
    }

  private:
    int x_;
    double y_;
};

/******************************************************************************/
/*                              concrete class 2                              */
/******************************************************************************/

class Class2 : public SuperClass {
    SERIALIZABLE_SUPER(SuperClass, std::string);

  public:
    explicit Class2(const std::string &name = "", int age = 0,
                    std::string str = "")
        : SuperClass(name, age), SERIALIZER(str_), str_(std::move(str)) {}

    /* accessors **************************************************************/
    void str(std::string str) { this->str_ = std::move(str); }
    [[nodiscard]] std::string str() const { return str_; }

    /* operator== *************************************************************/
    bool operator==(const SuperClass *other) const override {
        if (const auto *c = dynamic_cast<const Class2 *>(other)) {
            return str_ == c->str_ && SuperClass::operator==(other);
        }
        return false;
    }

  private:
    std::string str_;
};

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct SuperConvertor : public serializer::Convertor<SuperClass *> {
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
