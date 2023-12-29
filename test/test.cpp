#include "catch.hpp"
#include "test-classes/composed.hpp"
#include "test-classes/polymorphic.hpp"
#include "test-classes/simple.hpp"
#include "test-classes/withcontainer.hpp"
#include "test-classes/withpointers.hpp"
#include "test-classes/withstring.hpp"
#include <iostream>
#include <string>

/******************************************************************************/
/*                         tests with a simple class                          */
/******************************************************************************/

TEST_CASE("serialization/deserialisation on a SIMPLE CLASS") {
    Simple original(10, 20);
    Simple other(0, 0);
    std::string result;

    REQUIRE(original.getX() != other.getX());
    REQUIRE(original.getY() != other.getY());

    result = original.serialize();
    other.deserialize(result);

    // the serialisation and deserialisation work
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

TEST_CASE("serialization/deserialisation on a COMPOSED CLASS") {
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

TEST_CASE("serialization/deserialisation with STRING ATTRIBUTE") {
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
TEST_CASE("serialization/deserialisation with POINTERS ATTRIBUTE") {
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
TEST_CASE("serialization/deserialisation with ITERABLES ATTRIBUTE") {
    WithContainer original;
    WithContainer other;
    std::string result;

    // adding elements in containers
    for (int i = 0; i < 10; ++i) {
        original.addInt(i);
        original.addDouble(double(i));
        original.addSimple(Simple(i, 2 * i));
    }

    REQUIRE(original.getEmptyVec().size() == 0);
    REQUIRE(other.getEmptyVec().size() == 0);
    REQUIRE(original.getVec().size() == 10);
    REQUIRE(original.getLst().size() == 10);
    REQUIRE(original.getClassVec().size() == 10);
    REQUIRE(other.getVec().size() == 0);
    REQUIRE(other.getLst().size() == 0);
    REQUIRE(other.getClassVec().size() == 0);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getEmptyVec().size() == 0);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(original.getVec()[i] == other.getVec()[i]);
        REQUIRE(original.getClassVec()[i] == other.getClassVec()[i]);
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
    original.push_back(new Concrete2("hello world"));

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
