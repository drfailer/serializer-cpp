#include <iostream>
#include "serializable.hpp"

/******************************************************************************/
/*                                test classes                                */
/******************************************************************************/

/* Basic test class for the stringifier ***************************************/
class Test : public Serializable<int, int> {
public:
    Test(int _x = 0, int _y = 0) : serializable(x, y), x(_x), y(_y) { }
    Test(const Test& other): Test(other.x, other.y) { }
    ~Test() = default;

    Test operator=(const Test& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

private:
    int x;
    int y;
};

/* Test class that contains a stringifiable subclass **************************/
class TestComposed : public Serializable<Test, int, double> {
public:
    TestComposed(const Test& _t = Test(0, 0), int _z = 0, double _w = 0):
        serializable(t, z, w), t(_t), z(_z), w(_w) { }
    ~TestComposed() = default;

    /* accessors **************************************************************/
    void setZ(int newZ) { z = newZ; }
    void setW(double newW) { w = newW; }
    void setT(const Test& newT) { t = newT; }

private :
    Test t;
    int z;
    double w;
};

/******************************************************************************/
/*                                    main                                    */
/******************************************************************************/

void testContainer() {
    // create a simple container
    int x = 1;
    AttrContainer<int> c(x, "x");
    std::string serializedC = c.serialize();
    std::cout << serializedC << std::endl;

    // multple values container
    x = 3;
    int y = 2;
    AttrContainer<int, int> c2(x, y, "x, y");

    std::string serializedC2 = c2.serialize();
    std::cout << serializedC2 << std::endl;

    // deserialize works
    int cx = 0;
    AttrContainer<int> deserializedC(cx, "x");
    deserializedC.deserialize(serializedC);
    std::cout << "cx: " << cx << std::endl;

    cx = 0;
    int cy = 0;
    AttrContainer<int, int> deserializedC2(cx, cy, "x, y");
    deserializedC2.deserialize(serializedC2);
    std::cout << "cx: " << cx << std::endl;
    std::cout << "cy: " << cy << std::endl;
}

int main(int, char **) {
    Test test(1, 2);
    TestComposed testComposed(test, 1, 3.14);

    // the basic test works fine
    std::cout << test.serialize() << std::endl;

    // the stringifiable inner class is well printed
    std::cout << testComposed.serialize() << std::endl;

    testComposed.setZ(33);
    testComposed.setW(1.62);
    testComposed.setT(Test(3, 3));

    // thanks to the references, the elements are changed
    std::cout << testComposed.serialize() << std::endl;

    Test testCopy(0, 0);
    TestComposed testComposedCopy(testCopy, 0, 0);

    std::cout << "before deserialization:" << std::endl;
    std::cout << "testCopy: " << std::endl;
    std::cout << testCopy.serialize() << std::endl;

    std::cout << "testComposedCopy: " << std::endl;
    std::cout << testComposedCopy.serialize() << std::endl;

    testCopy.deserialize(test.serialize());
    testComposedCopy.deserialize(testComposed.serialize());

    std::cout << "after deserialization:" << std::endl;
    std::cout << "testCopy: " << std::endl;
    std::cout << testCopy.serialize() << std::endl;

    std::cout << "testComposedCopy: " << std::endl;
    std::cout << testComposedCopy.serialize() << std::endl;

    /* testContainer(); */

    return 0;
}
