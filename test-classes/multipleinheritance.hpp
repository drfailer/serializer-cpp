#ifndef MULTIPLE_INHERITANCE_HPP
#define MULTIPLE_INHERITANCE_HPP
#include "serializer/convertor.hpp"
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
    void setName(std::string name) { this->name = name; }
    std::string getName() const { return name; }
    void setAge(int age) { this->age = age; }
    int getAge() const { return age; }
    Mother(int _age = 0, const std::string &_name = "")
        : SERIALIZER(age, name), age(_age), name(_name) {}
    virtual ~Mother() = default;

    /* operator== *************************************************************/
    virtual bool operator==(const Mother* other) const {
        return age == other->age && name == other->name;
    }

  private:
    int age;
    std::string name;
};

/******************************************************************************/
/*                              daughter depth 1                              */
/******************************************************************************/

class Daughter1 : public Mother {
    SERIALIZABLE_SUPER(Mother, double);

  public:
    void setMoney(double money) { this->money = money; }
    double getMoney() const { return money; }
    Daughter1(int age = 0, const std::string &name = "", double _money = 0)
        : Mother(age, name), SERIALIZER(money), money(_money) {}
    ~Daughter1() = default;

    /* operator== *************************************************************/
    bool operator==(const Mother* other) const override {
        if (const Daughter1* d = dynamic_cast<const Daughter1*>(other)) {
            return Mother::operator==(other) && money == d->money;
        }
        return false;
    }

  private:
    double money;
};

/******************************************************************************/
/*                              daughter depth 2                              */
/******************************************************************************/

class Daughter2 : public Daughter1 {
    SERIALIZABLE_SUPER(Daughter1, std::string);

  public:
    Daughter2(int age = 0, const std::string &name = "", double money = 0,
              const std::string &_jobName = "")
        : Daughter1(age, name, money), SERIALIZER(jobName), jobName(_jobName) {}
    ~Daughter2() = default;

    /* operator== *************************************************************/
    bool operator==(const Mother* other) const override {
        if (const Daughter2* d = dynamic_cast<const Daughter2*>(other)) {
            return Daughter1::operator==(other) && jobName == d->jobName;
        }
        return false;
    }

  private:
    std::string jobName;
};

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct MIConvertor : public serializer::Convertor<Mother*> {
    HANDLE_POLYMORPHIC(Mother, Daughter1, Daughter2)
};

/******************************************************************************/
/*                                 container                                  */
/******************************************************************************/

class Collection {
    SERIALIZABLE_WITH_CONVERTOR(MIConvertor, std::vector<Mother*>);
  public:
    const std::vector<Mother *> &getElements() const { return elements; }
    void push_back(Mother *elt) { elements.push_back(elt); }
    Collection(): SERIALIZER(elements) {}
    ~Collection() = default;

  private:
    std::vector<Mother *> elements;
};

} // end namespace mi

#endif
