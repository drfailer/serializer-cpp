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
    SuperAbstract() = default;
    virtual ~SuperAbstract() = default;

    virtual void method() = 0;
    virtual bool operator==(const SuperAbstract *) const = 0;
};

/******************************************************************************/
/*                              concrete class 1                              */
/******************************************************************************/

class Concrete1 : public SuperAbstract {
    SERIALIZABLE_SUPER(SuperAbstract, int, double);

  public:
    explicit Concrete1(int x = 0, double y = 0) : SERIALIZER(x_, y_), x_(x), y_(y) {}

    /* accessors **************************************************************/
    void x(int x) { this->x_ = x; }
    void y(double y) { this->y_ = y; }
    [[nodiscard]] int x() const { return x_; }
    [[nodiscard]] double y() const { return y_; }

    /* method *****************************************************************/
    void method() override { std::cout << "Concrete1" << std::endl; }

    /* operator== *************************************************************/
    bool operator==(const SuperAbstract *other) const override {
        if (const auto *c = dynamic_cast<const Concrete1 *>(other)) {
            return x_ == c->x_ && y_ == c->y_;
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

class Concrete2 : public SuperAbstract {
    SERIALIZABLE_SUPER(SuperAbstract, std::string);

  public:
    explicit Concrete2(std::string str = "") : SERIALIZER(str_), str_(std::move(str)) {}

    /* accessors **************************************************************/
    void str(std::string str) { this->str_ = std::move(str); }
    [[nodiscard]] std::string str() const { return str_; }

    /* method *****************************************************************/
    void method() override { std::cout << "Concrete2" << std::endl; }

    /* operator== *************************************************************/
    bool operator==(const SuperAbstract *other) const override {
        if (const auto *c = dynamic_cast<const Concrete2 *>(other)) {
            return str_ == c->str_;
        }
        return false;
    }

  private:
    std::string str_;
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
