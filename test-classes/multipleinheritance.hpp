#ifndef MULTIPLE_INHERITANCE_HPP
#define MULTIPLE_INHERITANCE_HPP
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>
#include <string>
#include <vector>

namespace mi {

class Mother;
class Daughter1;
class Daughter2;
using MITable = serializer::tools::TypeTable<Mother, Daughter1, Daughter2>;
using MISerializer = serializer::Serializer<serializer::Bytes, MITable>;

/******************************************************************************/
/*                                mother class                                */
/******************************************************************************/

class Mother {
  public:
    explicit Mother(int _age = 0, std::string name = "")
        : age_(_age), name_(std::move(name)) {}
    virtual ~Mother() = default;

    VIRTUAL_SERIALIZE(MISerializer, age_, name_)

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
  public:
    explicit Daughter1(int age = 0, std::string name = "", double money = 0)
        : Mother(age, std::move(name)), money_(money) {}

    SERIALIZE_OVERRIDE(MISerializer, serializer::tools::super<Mother>(this),
                       money_)

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
  public:
    explicit Daughter2(int age = 0, const std::string &name = "",
                       double money = 0, std::string jobName = "")
        : Daughter1(age, name, money), jobName_(std::move(jobName)) {}

    SERIALIZE_OVERRIDE(MISerializer, serializer::tools::super<Daughter1>(this),
                       jobName_)

    /* accessors **************************************************************/
    void jobName(std::string jobName) { this->jobName_ = std::move(jobName); }
    [[nodiscard]] std::string const &jobName() const { return jobName_; }

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
/*                                 Collection                                 */
/******************************************************************************/

class Collection {
  public:
    SERIALIZE_CONV(MISerializer, elements_);

    /* accessors **************************************************************/
    [[nodiscard]] const std::vector<Mother *> &elements() const {
        return elements_;
    }
    void push_back(Mother *elt) { elements_.push_back(elt); }

  private:
    std::vector<Mother *> elements_;
};

} // end namespace mi

#endif
