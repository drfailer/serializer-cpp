#ifndef POLYMORPHIC_HPP
#define POLYMORPHIC_HPP
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include <vector>

class SuperClass;
class Class1;
class Class2;
using SuperTable = serializer::tools::TypeTable<SuperClass, Class1, Class2>;
using SuperSerializer = serializer::Serializer<serializer::Bytes, SuperTable>;

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperClass {
  public:
    explicit SuperClass(std::string _name = "", int _age = 0)
        : name_(std::move(_name)), age_(_age) {}
    virtual ~SuperClass() = default;

    virtual bool operator==(const SuperClass *other) const {
        return name_ == other->name_ && age_ == other->age_;
    }

    VIRTUAL_SERIALIZE(SuperSerializer, name_, age_);

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

  public:
    explicit Class1(std::string const &name = "", int age = 0, int x = 0,
                    double y = 0)
        : SuperClass(name, age), x_(x), y_(y) {}

    SERIALIZE_OVERRIDE(SuperSerializer,
                       serializer::tools::super<SuperClass>(this), x_, y_);

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
  public:
    explicit Class2(const std::string &name = "", int age = 0,
                    std::string str = "")
        : SuperClass(name, age), str_(std::move(str)) {}

    SERIALIZE_OVERRIDE(SuperSerializer,
                       serializer::tools::super<SuperClass>(this), str_);

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
/*                            abstract collection                             */
/******************************************************************************/

class SuperCollection {
  public:
    SuperCollection() {}
    ~SuperCollection() {
        for (auto elt : elements) {
            delete elt;
        }
    }

    SERIALIZE_CONV(SuperSerializer, elements);

    void push_back(SuperClass *element) { elements.push_back(element); }
    const std::vector<SuperClass *> &getElements() { return elements; }

  private:
    std::vector<SuperClass *> elements;
};

#endif
