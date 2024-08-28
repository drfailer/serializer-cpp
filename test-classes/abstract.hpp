#ifndef ABSTRACT_HPP
#define ABSTRACT_HPP
#include <iostream>
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include <vector>

class Concrete1;
class Concrete2;
using AbstractTable = serializer::tools::TypeTable<uint8_t, Concrete1, Concrete2>;

/******************************************************************************/
/*                               super abstract                               */
/******************************************************************************/

class SuperAbstract {
  public:
    SuperAbstract() = default;
    virtual ~SuperAbstract() = default;
    using MemT = serializer::Bytes;

    SERIALIZE_ABSTRACT(serializer::Bytes);

    virtual void method() = 0;
    virtual bool operator==(const SuperAbstract *) const = 0;
    virtual bool operator==(std::shared_ptr<SuperAbstract> const &) const = 0;
    virtual bool operator==(std::unique_ptr<SuperAbstract> const &) const = 0;
};

/******************************************************************************/
/*                              concrete class 1                              */
/******************************************************************************/

class Concrete1 : public SuperAbstract {

  public:
    explicit Concrete1(int x = 0, double y = 0) : x_(x), y_(y) {}

    SERIALIZE_OVERRIDE(serializer::Serializer<serializer::Bytes>,
                       serializer::tools::getId<Concrete1>(AbstractTable()), x_,
                       y_);

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

    bool
    operator==(std::shared_ptr<SuperAbstract> const &other) const override {
        if (const auto &c = std::dynamic_pointer_cast<const Concrete1>(other)) {
            return x_ == c->x_ && y_ == c->y_;
        }
        return false;
    }

    bool
    operator==(std::unique_ptr<SuperAbstract> const &other) const override {
        if (const auto *c = dynamic_cast<const Concrete1 *>(other.get())) {
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
  public:
    explicit Concrete2(std::string str = "") : str_(std::move(str)) {}

    SERIALIZE_OVERRIDE(serializer::Serializer<serializer::Bytes>,
                       serializer::tools::getId<Concrete2>(AbstractTable()),
                       str_);

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

    bool
    operator==(std::shared_ptr<SuperAbstract> const &other) const override {
        if (const auto &c = std::dynamic_pointer_cast<const Concrete2>(other)) {
            return str_ == c->str_;
        }
        return false;
    }

    bool
    operator==(std::unique_ptr<SuperAbstract> const &other) const override {
        if (const auto *c = dynamic_cast<const Concrete2 *>(other.get())) {
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
template <typename MemT>
struct AbstractSerializer
    : serializer::Serializer<MemT, POLYMORPHIC_TYPE(SuperAbstract)> {
    using serializer::Serializer<MemT,
                                POLYMORPHIC_TYPE(SuperAbstract)>::Serializer;
    HANDLE_POLYMORPHIC(AbstractTable, SuperAbstract, Concrete1, Concrete2)
};

/******************************************************************************/
/*                            abstract collection                             */
/******************************************************************************/

class AbstractCollection {
  public:
    AbstractCollection() = default;
    ~AbstractCollection() {
        for (auto elt : elements_) {
            delete elt;
        }
    }

    SERIALIZE_CONV(AbstractSerializer, elements_, elementsShared_,
                   elementsUnique_);

    /* accessors **************************************************************/
    void push_back(SuperAbstract *element) { elements_.push_back(element); }
    void push_back(std::shared_ptr<SuperAbstract> const &element) {
        elementsShared_.push_back(element);
    }
    void add_unique(std::unique_ptr<SuperAbstract> &&element) {
        elementsUnique_.push_back(std::move(element));
    }
    [[nodiscard]] const std::vector<SuperAbstract *> &elements() {
        return elements_;
    }
    [[nodiscard]] const std::vector<std::shared_ptr<SuperAbstract>> &
    elementsShared() {
        return elementsShared_;
    }
    [[nodiscard]] const std::vector<std::unique_ptr<SuperAbstract>> &
    elementsUnique() {
        return elementsUnique_;
    }

  private:
    std::vector<SuperAbstract *> elements_;
    std::vector<std::shared_ptr<SuperAbstract>> elementsShared_;
    std::vector<std::unique_ptr<SuperAbstract>> elementsUnique_;
};

#endif
