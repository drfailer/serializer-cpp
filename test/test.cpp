#include "catch.hpp"
#include "test-classes/abstract.hpp"
#include "test-classes/composed.hpp"
#include "test-classes/multipleinheritance.hpp"
#include "test-classes/polymorphic.hpp"
#include "test-classes/simple.hpp"
#include "test-classes/withMap.hpp"
#include "test-classes/withPair.hpp"
#include "test-classes/withtuple.hpp"
#include "test-classes/withSmartPtr.hpp"
#include "test-classes/withcontainer.hpp"
#include "test-classes/withconvertor.hpp"
#include "test-classes/withenums.hpp"
#include "test-classes/withpointers.hpp"
#include "test-classes/withset.hpp"
#include "test-classes/withstring.hpp"
#include <string>

/******************************************************************************/
/*                         tests with a simple class                          */
/******************************************************************************/

TEST_CASE("serialization/deserialization on a SIMPLE CLASS") {
    Simple original(10, 20, "hello \"world!\\");
    Simple other(0, 0);
    std::string result;

    REQUIRE(original.getX() != other.getX());
    REQUIRE(original.getY() != other.getY());
    REQUIRE(original.getStr() != other.getStr());

    result = original.serialize();
    other.deserialize(result);

    // the serialization and deserialization work
    REQUIRE(original.getX() == other.getX());
    REQUIRE(original.getY() == other.getY());

    original.setX(3); // modify an attribute

    REQUIRE(original.getX() != other.getX());

    result = original.serialize();
    other.deserialize(result);

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.getX() == other.getX());

    original.setX(28);
    original.setY(32);
    Simple copied = original;

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getX() == other.getX());
    REQUIRE(original.getY() == other.getY());
}

/******************************************************************************/
/*          tests with a class composed of a serializable subclass            */
/******************************************************************************/

TEST_CASE("serialization/deserialization on a COMPOSED CLASS") {
    Composed original(Simple(10, 20), 3, 3.14);
    Composed other(Simple(0, 0), 0, 0);
    std::string result;

    REQUIRE(original.getS().getX() != other.getS().getX());
    REQUIRE(original.getS().getY() != other.getS().getY());
    REQUIRE(original.getZ() != other.getZ());
    REQUIRE(original.getW() != other.getW());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getS().getX() == other.getS().getX());
    REQUIRE(original.getS().getY() == other.getS().getY());
    REQUIRE(original.getZ() == other.getZ());
    REQUIRE(original.getW() == other.getW());

    original.setW(1.618);
    original.setT(Simple(2, 2));

    REQUIRE(original.getS().getX() != other.getS().getX());
    REQUIRE(original.getS().getY() != other.getS().getY());
    REQUIRE(original.getW() != other.getW());

    result = original.serialize();
    other.deserialize(result);

    // again, we check if the references allow tracking of modifications
    REQUIRE(original.getS().getX() == other.getS().getX());
    REQUIRE(original.getS().getY() == other.getS().getY());
    REQUIRE(original.getZ() == other.getZ());
    REQUIRE(original.getW() == other.getW());
}

/******************************************************************************/
/*                             string seriasation                             */
/******************************************************************************/

TEST_CASE("serialization/deserialization with STRING ATTRIBUTE") {
    WithString original(2, "hello");
    WithString originalEmptyString(3, "");
    WithString other(0, "world");
    std::string result;

    REQUIRE(original.getX() != other.getX());
    REQUIRE(original.getStr() != other.getStr());
    REQUIRE(originalEmptyString.getX() != other.getX());
    REQUIRE(originalEmptyString.getStr() != other.getStr());

    // with non empty string
    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getX() == other.getX());
    REQUIRE(original.getStr() == other.getStr());

    // with empty string
    result = originalEmptyString.serialize();
    other.deserialize(result);

    REQUIRE(originalEmptyString.getX() == other.getX());
    REQUIRE(originalEmptyString.getStr() == other.getStr());

    originalEmptyString.setStr("world");

    result = originalEmptyString.serialize();
    other.deserialize(result);

    REQUIRE(originalEmptyString.getX() == other.getX());
    REQUIRE(originalEmptyString.getStr() == other.getStr());
}

/******************************************************************************/
/*                           pointer serialization                            */
/******************************************************************************/

/* IMPORTANT: this test should be ran with valgrind. */
TEST_CASE("serialization/deserialization with POINTERS ATTRIBUTE") {
    WithPointers original(new Simple(1, 2));
    WithPointers other(new Simple(0, 0));
    std::string result;

    REQUIRE(original.getClassPointer()->getX() !=
            other.getClassPointer()->getX());
    REQUIRE(original.getClassPointer()->getY() !=
            other.getClassPointer()->getY());
    REQUIRE(*original.getFundamentalPointer() ==
            *other.getFundamentalPointer()); // this one doesn't move here

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getClassPointer()->getX() ==
            other.getClassPointer()->getX());
    REQUIRE(original.getClassPointer()->getY() ==
            other.getClassPointer()->getY());
    REQUIRE(*original.getFundamentalPointer() ==
            *other.getFundamentalPointer());

    original.setClassPointer(new Simple(20, 30));
    original.setFundamentalPointer(new double(3.14));

    REQUIRE(original.getClassPointer()->getX() !=
            other.getClassPointer()->getX());
    REQUIRE(original.getClassPointer()->getY() !=
            other.getClassPointer()->getY());
    REQUIRE(*original.getFundamentalPointer() !=
            *other.getFundamentalPointer());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getClassPointer()->getX() ==
            other.getClassPointer()->getX());
    REQUIRE(original.getClassPointer()->getY() ==
            other.getClassPointer()->getY());
    REQUIRE(*original.getFundamentalPointer() ==
            *other.getFundamentalPointer());
}

/******************************************************************************/
/*                  serialization with a container attribute                  */
/******************************************************************************/

/*
 * NOTE: there is a probleme with precission for doubles.
 */
TEST_CASE("serialization/deserialization with ITERABLES ATTRIBUTE") {
    WithContainer original;
    WithContainer other;
    std::string result;

    // adding elements in containers
    for (int i = 0; i < 10; ++i) {
        original.addInt(i);
        original.addDouble(double(i));
        original.addSimple(Simple(i, 2 * i));
        original.addVec(std::vector<int>{1*i, 2*i, 3*i, 4*i, 5*i});
        original.addArr(i, i*2);
    }

    REQUIRE(original.getEmptyVec().size() == 0);
    REQUIRE(other.getEmptyVec().size() == 0);
    REQUIRE(original.getVec().size() == 10);
    REQUIRE(original.getLst().size() == 10);
    REQUIRE(original.getClassVec().size() == 10);
    REQUIRE(original.getArr().size() == 10);
    REQUIRE(other.getVec().size() == 0);
    REQUIRE(other.getLst().size() == 0);
    REQUIRE(other.getClassVec().size() == 0);
    REQUIRE(other.getVec2D().size() == 0);
    REQUIRE(other.getArr().size() == 10);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getEmptyVec().size() == 0);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(original.getVec()[i] == other.getVec()[i]);
        REQUIRE(original.getClassVec()[i] == other.getClassVec()[i]);
        REQUIRE(original.getArr()[i] == other.getArr()[i]);
        for (int j = 0; j < 5; ++j) {
            REQUIRE(original.getVec2D().at(i).at(j) == original.getVec2D().at(i).at(j));
        }
    }

    auto it = original.getLst().begin();
    for (double d : other.getLst()) {
        REQUIRE(d == *it++);
    }
}

/******************************************************************************/
/*                       implementation of a convertor                        */
/******************************************************************************/

TEST_CASE("implementing a convertor (polymorphic class serialization)") {
    AbstractCollection original;
    AbstractCollection other;
    std::string result;

    original.push_back(new Concrete1(1, 2.9));
    original.push_back(new Concrete2("hello \"test\" world"));

    REQUIRE(original.getElements().size() == 2);
    REQUIRE(other.getElements().size() == 0);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getElements().size() == original.getElements().size());
    auto it = other.getElements().begin();
    for (SuperAbstract *sa : original.getElements()) {
        REQUIRE(*sa == *it++);
    }
}

/******************************************************************************/
/*                                inheritance                                 */
/******************************************************************************/

TEST_CASE("serialize super class") {
    SuperCollection original;
    SuperCollection other;
    std::string result;
    Class1 *c1 = new Class1("John", 20, 1, 2.9);
    Class2 *c2 = new Class2("David", 30, "hello world");

    // test with super class serialization:

    result = c1->serialize();
    Class1 c11;
    c11.deserialize(result);
    REQUIRE(c11.serialize() == c1->serialize());

    result = c2->serialize();
    Class2 c22;
    c22.deserialize(result);
    REQUIRE(c22.serialize() == c2->serialize());

    original.push_back(c1);
    original.push_back(c2);

    REQUIRE(original.getElements().size() == 2);
    REQUIRE(other.getElements().size() == 0);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getElements().size() == original.getElements().size());
    auto it = other.getElements().begin();
    for (SuperClass *sa : original.getElements()) {
        REQUIRE(*sa == *it++);
    }
}

/******************************************************************************/
/*                                using files                                 */
/******************************************************************************/

TEST_CASE("serialization/deserialization in a FILE") {
    Simple original(10, 20);
    Simple other(0, 0);

    REQUIRE(original.getX() != other.getX());
    REQUIRE(original.getY() != other.getY());

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    // the serialization and deserialization work
    REQUIRE(original.getX() == other.getX());
    REQUIRE(original.getY() == other.getY());

    original.setX(3); // modify an attribute

    REQUIRE(original.getX() != other.getX());

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.getX() == other.getX());

    original.setX(28);
    original.setY(32);
    Simple copied = original;

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    REQUIRE(original.getX() == other.getX());
    REQUIRE(original.getY() == other.getY());
}

/******************************************************************************/
/*                            multiple inheritance                            */
/******************************************************************************/

TEST_CASE("multiple inheritance") {
    mi::Collection original;
    mi::Collection other;
    std::string result;
    mi::Daughter1 *c1 = new mi::Daughter1(10, "test1", 2.2);
    mi::Daughter2 *c2 = new mi::Daughter2(10, "test2", 2.2, "job");
    mi::Daughter2 *c3 = new mi::Daughter2(10, "test3", 2.2, "other job");

    // test with super class serialization:

    result = c1->serialize();
    mi::Daughter1 c11;
    c11.deserialize(result);
    REQUIRE(c11.serialize() == c1->serialize());

    result = c2->serialize();
    mi::Daughter2 c22;
    c22.deserialize(result);
    REQUIRE(c22.serialize() == c2->serialize());

    original.push_back(c1);
    original.push_back(c2);
    original.push_back(c3);

    REQUIRE(original.getElements().size() == 3);
    REQUIRE(other.getElements().size() == 0);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getElements().size() == original.getElements().size());
    auto it = other.getElements().begin();
    for (mi::Mother *m : original.getElements()) {
        REQUIRE(*m == *it++);
    }
}

/******************************************************************************/
/*                               smart poiters                                */
/******************************************************************************/

TEST_CASE("smart pointers") {
    WithSmartPtr original(1, 2.3, "hello");
    WithSmartPtr other;
    std::string result;

    REQUIRE(original.getIntPtr() != other.getIntPtr());
    REQUIRE(original.getDoublePtr() != other.getDoublePtr());
    REQUIRE(original.getOtherType() != other.getOtherType());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getIntPtr() == other.getIntPtr());
    REQUIRE(original.getDoublePtr() == other.getDoublePtr());
    REQUIRE(original.getOtherType() == other.getOtherType());
}

/******************************************************************************/
/*                                   enums                                    */
/******************************************************************************/

TEST_CASE("enums") {
    WithEnums original("str", SUNDAY, VISUAL, DndClasses::ROGUE);
    WithEnums other;
    std::string result;

    REQUIRE(original.getNormalType() != other.getNormalType());
    REQUIRE(original.getDay() != other.getDay());
    REQUIRE(original.getMode() != other.getMode());
    REQUIRE(original.getClass() != other.getClass());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getNormalType() == other.getNormalType());
    REQUIRE(original.getDay() == other.getDay());
    REQUIRE(original.getMode() == other.getMode());
    REQUIRE(original.getClass() == other.getClass());
}

/******************************************************************************/
/*                                   pairs                                    */
/******************************************************************************/

TEST_CASE("pairs") {
    std::vector<int> v = { 1, 2 ,3 ,4 };
    std::set<std::string> s = { "hello", "world" };
    WithPair original(1, 2, "hello", "world", Simple(10, 20),
                      Composed(Simple(10, 20), 3, 3.14), v, s);
    WithPair other;
    std::string result;

    REQUIRE(original.getIntPair().first != other.getIntPair().first);
    REQUIRE(original.getIntPair().second != other.getIntPair().second);
    REQUIRE(original.getStringPair().first != other.getStringPair().first);
    REQUIRE(original.getStringPair().first != other.getStringPair().first);
    REQUIRE(original.getObjPair().second != other.getObjPair().second);
    REQUIRE(original.getObjPair().second != other.getObjPair().second);
    REQUIRE(original.getContainerPair().first.size() != other.getContainerPair().first.size());
    REQUIRE(original.getContainerPair().second.size() != other.getContainerPair().second.size());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.getIntPair().first == other.getIntPair().first);
    REQUIRE(original.getIntPair().second == other.getIntPair().second);
    REQUIRE(original.getStringPair().first == other.getStringPair().first);
    REQUIRE(original.getStringPair().second == other.getStringPair().second);
    REQUIRE(original.getObjPair().second == other.getObjPair().second);
    REQUIRE(original.getObjPair().second == other.getObjPair().second);
    REQUIRE(original.getContainerPair().first.size() == other.getContainerPair().first.size());
    REQUIRE(original.getContainerPair().second.size() == other.getContainerPair().second.size());

    auto originalFirstIt = original.getContainerPair().first.begin();
    auto otherFirstIt = other.getContainerPair().first.begin();
    while (originalFirstIt != original.getContainerPair().first.end()) {
        REQUIRE(*originalFirstIt++ == *otherFirstIt++);
    }

    auto originalSecondIt = original.getContainerPair().second.begin();
    auto otherSecondIt = other.getContainerPair().second.begin();
    while (originalSecondIt != original.getContainerPair().second.end()) {
        REQUIRE(*originalSecondIt++ == *otherSecondIt++);
    }
}

/******************************************************************************/
/*                                   tuples                                   */
/******************************************************************************/

TEST_CASE("tuples") {
    std::vector<int> v = { 1, 2 ,3 ,4 };
    std::set<std::string> s = { "hello", "world" };
    std::map<std::string, std::string> m = { {"foo", "barr"}, {"hello", "wolrd"} };
    WithTuple original(1, 2, 1.618, "hello", "world", "!", Simple(10, 20),
                      Composed(Simple(10, 20), 3, 3.14), v, s, m);
    WithTuple other;
    std::string result;

    REQUIRE(std::get<0>(original.getNumberTuple()) != std::get<0>(other.getNumberTuple()));
    REQUIRE(std::get<1>(original.getNumberTuple()) != std::get<1>(other.getNumberTuple()));
    REQUIRE(std::get<2>(original.getNumberTuple()) != std::get<2>(other.getNumberTuple()));

    REQUIRE(std::get<0>(original.getStringTuple()) != std::get<0>(other.getStringTuple()));
    REQUIRE(std::get<1>(original.getStringTuple()) != std::get<1>(other.getStringTuple()));
    REQUIRE(std::get<2>(original.getStringTuple()) != std::get<2>(other.getStringTuple()));

    REQUIRE(std::get<0>(original.getObjTuple()) != std::get<0>(other.getObjTuple()));
    REQUIRE(std::get<1>(original.getObjTuple()) != std::get<1>(other.getObjTuple()));

    REQUIRE(std::get<0>(original.getContainerTuple()).size() != std::get<0>(other.getContainerTuple()).size());
    REQUIRE(std::get<1>(original.getContainerTuple()).size() != std::get<1>(other.getContainerTuple()).size());
    REQUIRE(std::get<2>(original.getContainerTuple()).size() != std::get<2>(other.getContainerTuple()).size());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(std::get<0>(original.getNumberTuple()) == std::get<0>(other.getNumberTuple()));
    REQUIRE(std::get<1>(original.getNumberTuple()) == std::get<1>(other.getNumberTuple()));
    REQUIRE(std::get<2>(original.getNumberTuple()) == std::get<2>(other.getNumberTuple()));

    REQUIRE(std::get<0>(original.getStringTuple()) == std::get<0>(other.getStringTuple()));
    REQUIRE(std::get<1>(original.getStringTuple()) == std::get<1>(other.getStringTuple()));
    REQUIRE(std::get<2>(original.getStringTuple()) == std::get<2>(other.getStringTuple()));

    REQUIRE(std::get<0>(original.getObjTuple()) == std::get<0>(other.getObjTuple()));
    REQUIRE(std::get<1>(original.getObjTuple()) == std::get<1>(other.getObjTuple()));

    REQUIRE(std::get<0>(original.getContainerTuple()).size() == std::get<0>(other.getContainerTuple()).size());
    REQUIRE(std::get<1>(original.getContainerTuple()).size() == std::get<1>(other.getContainerTuple()).size());
    REQUIRE(std::get<2>(original.getContainerTuple()).size() == std::get<2>(other.getContainerTuple()).size());

    auto originalFirstIt = std::get<0>(original.getContainerTuple()).begin();
    auto otherFirstIt = std::get<0>(other.getContainerTuple()).begin();
    while (originalFirstIt != std::get<0>(original.getContainerTuple()).end()) {
        REQUIRE(*originalFirstIt++ == *otherFirstIt++);
    }

    auto originalSecondIt = std::get<1>(original.getContainerTuple()).begin();
    auto otherSecondIt = std::get<1>(other.getContainerTuple()).begin();
    while (originalSecondIt != std::get<1>(original.getContainerTuple()).end()) {
        REQUIRE(*originalSecondIt++ == *otherSecondIt++);
    }

    auto originalThirdIt = std::get<2>(original.getContainerTuple()).begin();
    auto otherThirdIt = std::get<2>(other.getContainerTuple()).begin();
    while (originalThirdIt != std::get<2>(original.getContainerTuple()).end()) {
        REQUIRE(*originalThirdIt++ == *otherThirdIt++);
    }
}

/******************************************************************************/
/*                                 convertor                                  */
/******************************************************************************/

TEST_CASE("serialize unknown type") {
    WithConvertor original;
    WithConvertor other;
    std::string result;

    original.addInt(1);
    original.addInt(2);
    original.addInt(3);
    original.addUnknown(Unknown(1));
    original.addUnknown(Unknown(2));
    original.addUnknown(Unknown(3));

    REQUIRE(original.getInts().size() != 0);
    REQUIRE(original.getUnknowns().size() != 0);
    REQUIRE(other.getInts().size() == 0);
    REQUIRE(other.getUnknowns().size() == 0);

    std::cout << "----" << std::endl;
    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getInts().size() == original.getInts().size());
    REQUIRE(other.getUnknowns().size() == original.getUnknowns().size());

    for (std::size_t i = 0; i < other.getInts().size(); ++i) {
        REQUIRE(other.getInts().at(i) == original.getInts().at(i));
    }

    for (std::size_t i = 0; i < other.getInts().size(); ++i) {
        REQUIRE(other.getUnknowns().at(i) == original.getUnknowns().at(i));
    }
}

/******************************************************************************/
/*                                    map                                     */
/******************************************************************************/

TEST_CASE("map") {
    WithMap original;
    WithMap other;
    std::string result;

    REQUIRE(original.getMap().size() == 0);
    REQUIRE(other.getMap().size() == 0);

    original.insert("hello", "pouf");
    original.insert("world", "pouf");

    REQUIRE(original.getMap().size() == 2);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getMap().size() == original.getMap().size());
    REQUIRE(other.getMap().at("hello") == "pouf");
    REQUIRE(other.getMap().at("world") == "pouf");
}

/******************************************************************************/
/*                                    set                                     */
/******************************************************************************/

TEST_CASE("set") {
    WithSet original;
    WithSet other;
    std::string result;

    REQUIRE(original.getSet().size() == 0);
    REQUIRE(other.getSet().size() == 0);

    original.insert("hello");
    original.insert("world");

    REQUIRE(original.getSet().size() == 2);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getSet().size() == original.getSet().size());
    auto original_it = original.getSet().begin();
    for (auto other_it = other.getSet().begin();
         other_it != other.getSet().end(); other_it++) {
        REQUIRE(*other_it == *original_it);
        original_it++;
    }
}
