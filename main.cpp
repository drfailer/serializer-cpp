#include <iostream>
#include "stringifier.hpp"
#include "stringifiable.hpp"

/******************************************************************************/
/*                                test classes                                */
/******************************************************************************/

/* Basic test class for the stringifier. **************************************/
class Test : public Stringifiable {
  public:
    Test(int _x = 0, int _y = 0) : x(_x), y(_y), constructStringifier(x, y) {}
    Test(const Test& other): Test(other.x, other.y) {}
    ~Test() = default;

    Test operator=(const Test& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    // generate the toString function
    genToString()

  private:
    int x;
    int y;
    genStringifier(int, int)
};

/* Test class that contains a stringifiable subclass **************************/
class TestComposed {
  public:
    TestComposed(Test _t = Test(0, 0), int _z = 0, double _w = 0):
        constructStringifier(t, z, w), t(_t), z(_z), w(_w) {}
    ~TestComposed() = default;

    // generate the toString function
    stringifier(Test, int, double) // declare both the stringifier and the
                                   // toString function

    /* accessors **************************************************************/
    void setZ(int newZ) { z = newZ; }
    void setW(double newW) { w = newW; }
    void setT(Test newT) { t = newT; }

  private :
    Test t;
    int z;
    double w;
};

/******************************************************************************/
/*                                    main                                    */
/******************************************************************************/

int main(int, char **) {
    Test test(1, 2);
    TestComposed testComposed(test, 1, 3.14);

    // the basic test works fine
    std::cout << test.toString() << std::endl;

    // the stringifiable inner class is well printed
    std::cout << testComposed.toString() << std::endl;

    testComposed.setZ(33);
    testComposed.setW(1.62);
    testComposed.setT(Test(3, 3));

    // thanks to the references, the elements are changed
    std::cout << testComposed.toString() << std::endl;
    return 0;
}
