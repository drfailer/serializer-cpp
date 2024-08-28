#ifndef POLYMORPHIC_HPP
#define POLYMORPHIC_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>
#include <vector>

class SuperClass;
class Class1;
class Class2;
using SuperTable = serializer::tools::TypeTable<char, SuperClass, Class1, Class2>;

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

    VIRTUAL_SERIALIZE(serializer::Convertor<serializer::default_mem_type>,
                      serializer::tools::getId<SuperClass>(SuperTable()), name_,
                      age_);

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

    SERIALIZE_OVERRIDE(serializer::Convertor<serializer::default_mem_type>,
                       serializer::tools::getId<Class1>(SuperTable()),
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

    SERIALIZE_OVERRIDE(serializer::Convertor<serializer::default_mem_type>,
                       serializer::tools::getId<Class2>(SuperTable()),
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
/*                                 convertor                                  */
/******************************************************************************/

template <typename MemT>
struct SuperConvertor : serializer::Convertor<MemT, POLYMORPHIC_TYPE(SuperClass)> {
  using serializer::Convertor<MemT, POLYMORPHIC_TYPE(SuperClass)>::Convertor;
  HANDLE_POLYMORPHIC(SuperTable, SuperClass, Class1, Class2);
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

    SERIALIZE_CONV(SuperConvertor, elements);

    void push_back(SuperClass *element) { elements.push_back(element); }
    const std::vector<SuperClass *> &getElements() { return elements; }

  private:
    std::vector<SuperClass *> elements;
};

#endif
