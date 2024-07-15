#ifndef MULTIPLE_INHERITANCE_HPP
#define MULTIPLE_INHERITANCE_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/serializable.hpp"
#include "serializer/serializer.hpp"
#include <string>
#include <vector>

namespace mi {

/******************************************************************************/
/*                                mother class                                */
/******************************************************************************/

class Mother {
    SERIALIZABLE_POLYMORPHIC(int, std::string);

  public:
    explicit Mother(int _age = 0, std::string name = "")
        : SERIALIZER(age_, name_), age_(_age), name_(std::move(name)) {}
    virtual ~Mother() = default;

    /* accessors **************************************************************/
    void name(std::string name) { this->name_ = std::move(name); }
    void age(int age) { this->age_ = age; }
    [[nodiscard]] std::string name() const { return name_; }
    [[nodiscard]] int age() const { return age_; }

    /* operator== *************************************************************/
    virtual bool operator==(const Mother *other) const {
        return age_ == other->age_ && name_ == other->name_;
    }

  private:
    int age_;
    std::string name_;
};

/******************************************************************************/
/*                              daughter depth 1                              */
/******************************************************************************/

class Daughter1 : public Mother {
    SERIALIZABLE_SUPER(Mother, double);

  public:
    explicit Daughter1(int age = 0, std::string name = "", double money = 0)
        : Mother(age, std::move(name)), SERIALIZER(money_), money_(money) {}

    /* accessors **************************************************************/
    void money(double money) { this->money_ = money; }
    [[nodiscard]] double money() const { return money_; }

    /* operator== *************************************************************/
    bool operator==(const Mother *other) const override {
        if (const auto *d = dynamic_cast<const Daughter1 *>(other)) {
            return Mother::operator==(other) && money_ == d->money_;
        }
        return false;
    }

  private:
    double money_;
};

/******************************************************************************/
/*                              daughter depth 2                              */
/******************************************************************************/

class Daughter2 : public Daughter1 {
    SERIALIZABLE_SUPER(Daughter1, std::string);

  public:
    explicit Daughter2(int age = 0, const std::string &name = "",
                       double money = 0, std::string jobName = "")
        : Daughter1(age, name, money), SERIALIZER(jobName_),
          jobName_(std::move(jobName)) {}

    /* operator== *************************************************************/
    bool operator==(const Mother *other) const override {
        if (const auto *d = dynamic_cast<const Daughter2 *>(other)) {
            return Daughter1::operator==(other) && jobName_ == d->jobName_;
        }
        return false;
    }

  private:
    std::string jobName_;
};

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct MIConvertor : public serializer::Convertor<POLYMORPHIC_TYPE(Mother)> {
    HANDLE_POLYMORPHIC(Mother, Daughter1, Daughter2)
};

/******************************************************************************/
/*                                 members_                                  */
/******************************************************************************/

class Collection {
    SERIALIZABLE_WITH_CONVERTOR(MIConvertor, std::vector<Mother *>);

  public:
    Collection() : SERIALIZER(elements_) {}
    ~Collection() = default;

    /* accessors **************************************************************/
    [[nodiscard]] const std::vector<Mother *> &elements() const { return elements_; }
    void push_back(Mother *elt) { elements_.push_back(elt); }

  private:
    std::vector<Mother *> elements_;
};

} // end namespace mi

#endif
