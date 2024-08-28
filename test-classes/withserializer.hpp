#ifndef WITHCONVERTOR_HPP
#define WITHCONVERTOR_HPP
#include <serializer/serialize.hpp>
#include <serializer/tools/macros.hpp>
#include <string>
#include <vector>

/******************************************************************************/
/*                                   types                                    */
/******************************************************************************/

class Unknown {
  public:
    explicit Unknown(int x) : x_(x) {}
    Unknown() = default;
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

template <typename MemT>
struct UnknownSerializer : serializer::Serializer<MemT, Unknown> {
    using serializer::Serializer<MemT, Unknown>::Serializer;
    using byte_type = serializer::Serializer<MemT, Unknown>::byte_type;

    constexpr void serialize(const Unknown &u) override {
        int i = u.x();
        this->append(std::bit_cast<const byte_type *>(&i), sizeof(i));
    }

    constexpr void deserialize(Unknown &elt) override {
        int x;
        this->deserialize_(x);
        elt.x(x);
    }
};

/******************************************************************************/
/*                               with convertor                               */
/******************************************************************************/

class WithSerializer {
  public:
    SERIALIZE_CONV(UnknownSerializer, ints, unknowns);

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
