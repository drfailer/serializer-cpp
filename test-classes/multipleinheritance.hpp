#ifndef MULTIPLE_INHERITANCE_HPP
#define MULTIPLE_INHERITANCE_HPP
#include <serializer/serialize.hpp>
#include <string>
#include <vector>

namespace mi {

/******************************************************************************/
/*                                mother class                                */
/******************************************************************************/

class Mother {
  public:
    explicit Mother(int _age = 0, std::string name = "")
        : age_(_age), name_(std::move(name)) {}
    virtual ~Mother() = default;

    virtual constexpr size_t serialize(serializer::default_mem_type &mem, size_t pos = 0) const {
        return serializer ::serialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, age_, name_);
    }
    virtual constexpr size_t deserialize(serializer::default_mem_type &mem, size_t pos = 0) {
        return serializer ::deserialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, age_, name_);
    };

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

    constexpr size_t serialize(serializer::default_mem_type &mem, size_t pos = 0) const override {
        return serializer ::serialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, serializer ::super<Mother>(this), money_);
    }
    constexpr size_t deserialize(serializer::default_mem_type &mem, size_t pos = 0) override {
        return serializer ::deserialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, serializer ::super<Mother>(this), money_);
    }

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
        : Daughter1(age, name, money),
          jobName_(std::move(jobName)) {}

    constexpr size_t serialize(serializer::default_mem_type &mem, size_t pos = 0) const override {
        return serializer ::serialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, serializer ::super<Daughter1>(this), jobName_);
    }
    constexpr size_t deserialize(serializer::default_mem_type &mem, size_t pos = 0) override {
        return serializer ::deserialize<serializer ::Convertor<decltype(mem)>>(
            mem, pos, serializer ::super<Daughter1>(this), jobName_);
    }

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

template <typename MemT>
struct MIConvertor : serializer::Convertor<MemT, Mother *> {
    using serializer::Convertor<MemT, Mother *>::Convertor;
    using byte_type = serializer::Convertor<MemT, Mother *>::byte_type;
    // todo: using id_table = ?;
    // TODO
    constexpr void deserialize(Mother *&elt) override {
        // note: we can serialize the id here as well
        this->serialize(elt);
    }
    void serialize(Mother *const &elt) override {
        // TODO
        uint8_t id;
        this->deserialize_id(id);
        this->deserialize(serializer::polymorphic_cast<id_table>(elt, id));
    }
};

/******************************************************************************/
/*                                 members_                                  */
/******************************************************************************/

class Collection {
  public:
    SERIALIZE(elements_);

    /* accessors **************************************************************/
    [[nodiscard]] const std::vector<Mother *> &elements() const { return elements_; }
    void push_back(Mother *elt) { elements_.push_back(elt); }

  private:
    std::vector<Mother *> elements_;
};

} // end namespace mi

#endif
