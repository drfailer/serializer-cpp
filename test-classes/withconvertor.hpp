#ifndef WITHCONVERTOR_HPP
#define WITHCONVERTOR_HPP
#include "serializer/convertor/convertor.hpp"
#include "serializer/serializable.hpp"
#include "serializer/serializer.hpp"
#include <sstream>
#include <string>
#include <vector>

/******************************************************************************/
/*                                   types                                    */
/******************************************************************************/

class Unknown {
  public:
    explicit Unknown(int x) : x_(x) {}
    ~Unknown() = default;

    /* accessors **************************************************************/
    void x(int x) { this->x_ = x; }
    [[nodiscard]] int x() const { return x_; }

  private:
    int x_;
};

inline bool operator==(const Unknown &lhs, const Unknown &rhs) {
    return lhs.x() == rhs.x();
}

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct UnknownConvertor : public serializer::Convertor<Unknown> {
    std::string &serialize(const Unknown &u, std::string &str) const override {
        int i = u.x();
        str = str.append(reinterpret_cast<char *>(&i), sizeof(i));
        return str;
    }

    Unknown deserialize(std::string_view &str, Unknown &) override {
        int x = Convertor::deserialize_(str, x);
        return Unknown(x);
    }
};

/******************************************************************************/
/*                               with convertor                               */
/******************************************************************************/

class WithConvertor {
    SERIALIZABLE_WITH_CONVERTOR(UnknownConvertor, std::vector<int>,
                                std::vector<Unknown>);

  public:
    WithConvertor() : SERIALIZER(ints, unknowns) {}
    ~WithConvertor() = default;

    /* accessors **************************************************************/
    [[nodiscard]] const std::vector<Unknown> &getUnknowns() const {
        return unknowns;
    }
    [[nodiscard]] const std::vector<int> &getInts() const { return ints; }
    void addInt(int i) { ints.push_back(i); }
    void addUnknown(const Unknown &u) { unknowns.push_back(u); }

  private:
    std::vector<int> ints;
    std::vector<Unknown> unknowns;
};

#endif
