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
class TestComposed : public Serializable<Test, int, double, std::string, int*, double*, Test*> {
public:
    TestComposed(const Test& _t = Test(0, 0), int _z = 0, double _w = 0, const std::string& _str = ""):
        serializable(t, z, w, str, simple_nullptr, simple_ptr, test_ptr),
        t(_t), z(_z), w(_w), str(_str) {
        simple_ptr = new double;
        *simple_ptr = 1.9;
    }
    ~TestComposed() {
        delete simple_ptr;
        delete test_ptr;
    }

    /* accessors **************************************************************/
    void setZ(int newZ) { z = newZ; }
    void setW(double newW) { w = newW; }
    void setT(const Test& newT) { t = newT; }
    void setSimple_ptr(double val) { *simple_ptr = val; }
    void setTest_ptr(Test* test) { test_ptr = test; }

private :
    Test t;
    int z;
    double w;
    std::string str;
    int *simple_nullptr = nullptr;
    double *simple_ptr = nullptr;
    Test *test_ptr = nullptr;
};

/******************************************************************************/
/*                                    main                                    */
/******************************************************************************/

int main(int, char **) {
    Test test(1, 2);
    TestComposed testComposed(test, 1, 3.14, "hello world");

    // the basic test works fine
    std::cout << test.serialize() << std::endl;

    // the stringifiable inner class is well printed
    std::cout << testComposed.serialize() << std::endl;

    testComposed.setZ(33);
    testComposed.setW(1.62);
    testComposed.setT(Test(38, 427));
    testComposed.setSimple_ptr(9.5);
    testComposed.setTest_ptr(new Test(3, 8));

    // thanks to the references, the elements are changed
    std::cout << testComposed.serialize() << std::endl;

    // testing the deserialization using other objects
    Test testCopy(0, 0);
    TestComposed testComposedCopy(testCopy, 0, 0);

    // first display, we should have the default values
    std::cout << "before deserialization:" << std::endl;
    std::cout << "testCopy: " << testCopy.serialize() << std::endl;
    std::cout << "testComposedCopy: " << testComposedCopy.serialize() << std::endl;

    testCopy.deserialize(test.serialize());
    testComposedCopy.deserialize(testComposed.serialize());

    // second display, the attributes in new objects have the same values as the others
    std::cout << "after deserialization:" << std::endl;
    std::cout << "testCopy: " << testCopy.serialize() << std::endl;
    std::cout << "testComposedCopy: " << testComposedCopy.serialize() << std::endl;

    return 0;
}
