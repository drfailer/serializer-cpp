#ifndef WITHCONVERTOR_HPP
#define WITHCONVERTOR_HPP
#include "serializer/convertor.hpp"
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
    void setX(int x) { this->x = x; }
    int getX() const { return x; }
    Unknown(int x) : x(x) {}
    ~Unknown() = default;

  private:
    int x;
};

inline bool operator==(const Unknown& lhs, const Unknown& rhs) {
    return lhs.getX() == rhs.getX();
}

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

struct TestConvertor {
    serialize_custom_type(const Unknown &u) {
        std::ostringstream oss;
        oss << u.getX();
//        int i = u.getX();
//        str.append(reinterpret_cast<char*>(&i), sizeof(i));
        return oss.str();
    }

    deserialize_custom_type(Unknown, str) {
        std::istringstream iss((std::string(str)));
        int x;
        iss >> x;
        return Unknown(x);
    }

    CONVERTOR;
};

/******************************************************************************/
/*                               with convertor                               */
/******************************************************************************/

class WithConvertor {
    SERIALIZABLE_WITH_CONVERTOR(TestConvertor, std::vector<int>, std::vector<Unknown>);
  public:
    const std::vector<Unknown>& getUnknowns() const { return unknowns; }
    const std::vector<int>& getInts() const { return ints; }
    void addInt(int i) { ints.push_back(i); }
    void addUnknown(const Unknown& u) { unknowns.push_back(u); }
    WithConvertor() : SERIALIZER(ints, unknowns) {}
    ~WithConvertor() = default;

  private:
    std::vector<int> ints;
    std::vector<Unknown> unknowns;
};

#endif
