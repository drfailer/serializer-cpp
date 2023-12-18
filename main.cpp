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
    AttrContainer<int> c = { .name = "x", .reference = x };
    std::string serializedC = c.serialize();
    std::cout << serializedC << std::endl;

    // multple values container
    x = 3;
    int y = 2;
    AttrContainer<int, int> c2 = { .name = "x", .reference = x, .next = { .name = "y", .reference = y } };

    std::string serializedC2 = c2.serialize();
    std::cout << serializedC2 << std::endl;

    // deserialize works
    int cx = 0;
    AttrContainer<int> deserializedC = { .name = "x", .reference = cx };
    deserializedC.deserialize(serializedC);
    std::cout << "cx: " << cx << std::endl;

    cx = 0;
    int cy = 0;
    AttrContainer<int, int> deserializedC2 = { .name = "x", .reference = cx, .next = { .name = "y", .reference = cy } };
    deserializedC2.deserialize(serializedC2);
    std::cout << "cx: " << cx << std::endl;
    std::cout << "cy: " << cy << std::endl;
}

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

    testContainer();

    return 0;
}
