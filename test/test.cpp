#include "catch.hpp"
#include <chrono>
#include <iostream>
#include <string>

#define TEST_SIMPLE
#define TEST_COMPOSED
#define TEST_STRING
#define TEST_POINTERS
#define TEST_CONTAINERS
#define TEST_POLYMORPHIC
#define TEST_SUPER
#define TEST_INHERITANCE
#define TEST_SMART_PTR
#define TEST_ENUMS
#define TEST_PAIR
#define TEST_TUPLE
#define TEST_SERIALIZER
#define TEST_MAP
#define TEST_SET
#define TEST_CSTRUCT
#define TEST_FUNCTION
#define TEST_STATIC_ARRAYS
#define TEST_DYNAMIC_ARRAYS
#define TEST_TREE
#define TEST_HH

/******************************************************************************/
/*                         tests with a simple class                          */
/******************************************************************************/

#ifdef TEST_SIMPLE
#include "test-classes/simple.hpp"
TEST_CASE("serialization/deserialization on a SIMPLE CLASS") {
    Simple original(10, 20, "hello \"world!\\");
    Simple other(0, 0);
    serializer::default_mem_type result;

    REQUIRE(original.x() != other.x());
    REQUIRE(original.y() != other.y());
    REQUIRE(original.str() != other.str());

    original.serialize(result);
    other.deserialize(result);

    // the serialization and deserialization work
    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());

    original.x(3); // modify an attribute

    REQUIRE(original.x() != other.x());

    original.serialize(result);
    other.deserialize(result);

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.x() == other.x());

    original.x(28);
    original.y(32);
    Simple copied = original;

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());
}

TEST_CASE("bind serialize on a SIMPLE CLASS") {
    Simple original(10, 20, "hello \"world!\\");
    Simple other(0, 0);
    serializer::default_mem_type result;

    auto serializeOrigin = serializer::bindSerialize(original, &Simple::getX, &Simple::getY, &Simple::getStr);
    auto deserializeOther = serializer::bindDeserialize(other, &Simple::getX, &Simple::getY, &Simple::getStr);

    REQUIRE(original.x() != other.x());
    REQUIRE(original.y() != other.y());
    REQUIRE(original.str() != other.str());

    serializeOrigin(result);
    deserializeOther(result);

    // the serialization and deserialization work
    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());

    original.x(3); // modify an attribute

    REQUIRE(original.x() != other.x());

    serializeOrigin(result);
    deserializeOther(result);

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.x() == other.x());

    original.x(28);
    original.y(32);
    Simple copied = original;

    serializeOrigin(result);
    deserializeOther(result);

    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());
}
#endif

/******************************************************************************/
/*          tests with a class composed of a serializable subclass            */
/******************************************************************************/

#ifdef TEST_COMPOSED
#include "test-classes/composed.hpp"
TEST_CASE("serialization/deserialization on a COMPOSED CLASS") {
    Composed original(Simple(10, 20), 3, 3.14);
    Composed other(Simple(0, 0), 0, 0);
    serializer::default_mem_type result;

    REQUIRE(original.s().x() != other.s().x());
    REQUIRE(original.s().y() != other.s().y());
    REQUIRE(original.z() != other.z());
    REQUIRE(original.w() != other.w());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.s().x() == other.s().x());
    REQUIRE(original.s().y() == other.s().y());
    REQUIRE(original.z() == other.z());
    REQUIRE(original.w() == other.w());

    original.w(1.618);
    original.s(Simple(2, 2));

    REQUIRE(original.s().x() != other.s().x());
    REQUIRE(original.s().y() != other.s().y());
    REQUIRE(original.w() != other.w());

    original.serialize(result);
    other.deserialize(result);

    // again, we check if the references allow tracking of modifications
    REQUIRE(original.s().x() == other.s().x());
    REQUIRE(original.s().y() == other.s().y());
    REQUIRE(original.z() == other.z());
    REQUIRE(original.w() == other.w());
}
#endif

/******************************************************************************/
/*                            string serialization                            */
/******************************************************************************/

#ifdef TEST_STRING
#include "test-classes/withstring.hpp"
TEST_CASE("serialization/deserialization with STRING ATTRIBUTE") {
    WithString original(2, "hello");
    WithString originalEmptyString(3, "");
    WithString other(0, "world");
    serializer::default_mem_type result;

    REQUIRE(original.x() != other.x());
    REQUIRE(original.str() != other.str());
    REQUIRE(originalEmptyString.x() != other.x());
    REQUIRE(originalEmptyString.str() != other.str());

    // with non-empty string
    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.x() == other.x());
    REQUIRE(original.str() == other.str());

    // with empty string
    originalEmptyString.serialize(result);
    other.deserialize(result);

    REQUIRE(originalEmptyString.x() == other.x());
    REQUIRE(originalEmptyString.str() == other.str());

    originalEmptyString.str("world");

    originalEmptyString.serialize(result);
    other.deserialize(result);

    REQUIRE(originalEmptyString.x() == other.x());
    REQUIRE(originalEmptyString.str() == other.str());
}
#endif

/******************************************************************************/
/*                           pointer serialization                            */
/******************************************************************************/

#ifdef TEST_POINTERS
#include "test-classes/withpointers.hpp"
/* IMPORTANT: this test should be run with valgrind. */
TEST_CASE("serialization/deserialization with POINTERS ATTRIBUTE") {
    WithPointers original(new Simple(1, 2));
    WithPointers other(new Simple(0, 0));
    serializer::default_mem_type result;

    REQUIRE(original.classPointer()->x() != other.classPointer()->x());
    REQUIRE(original.classPointer()->y() != other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() ==
            *other.fundamentalPointer()); // this one doesn't move here

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.classPointer()->x() == other.classPointer()->x());
    REQUIRE(original.classPointer()->y() == other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() == *other.fundamentalPointer());

    original.classPointer(new Simple(20, 30));
    original.fundamentalPointer(new double(3.14));

    REQUIRE(original.classPointer()->x() != other.classPointer()->x());
    REQUIRE(original.classPointer()->y() != other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() != *other.fundamentalPointer());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.classPointer()->x() == other.classPointer()->x());
    REQUIRE(original.classPointer()->y() == other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() == *other.fundamentalPointer());
}
#endif

/******************************************************************************/
/*                  serialization with a members_ attribute                  */
/******************************************************************************/

/*
 * NOTE: there is a problem with precision for doubles.
 */
#ifdef TEST_CONTAINERS
#include "test-classes/withcontainer.hpp"
TEST_CASE("serialization/deserialization with ITERABLES ATTRIBUTE") {
    WithContainer original;
    WithContainer other;
    serializer::default_mem_type result;

    // adding elements_ in containers
    for (int i = 0; i < 10; ++i) {
        original.addInt(i);
        original.addDouble(double(i));
        original.addSimple(Simple(i, 2 * i));
        original.addVec(std::vector<int>{1 * i, 2 * i, 3 * i, 4 * i, 5 * i});
        original.addArr(i, i * 2);
        if (i % 2) {
            original.addArrPtr(i, nullptr);
        } else {
            original.addArrPtr(i, new int{i});
        }
        original.addArrSimple(i, Simple(i, i * 2, "test"));
    }

    REQUIRE(original.getEmptyVec().empty());
    REQUIRE(other.getEmptyVec().empty());
    REQUIRE(original.getVec().size() == 10);
    REQUIRE(original.getLst().size() == 10);
    REQUIRE(original.getClassVec().size() == 10);
    REQUIRE(original.getArr().size() == 10);
    REQUIRE(original.getArrPtr().size() == 10);
    REQUIRE(original.getArrSimple().size() == 10);
    REQUIRE(other.getVec().empty());
    REQUIRE(other.getLst().empty());
    REQUIRE(other.getClassVec().empty());
    REQUIRE(other.getVec2D().empty());
    REQUIRE(other.getArr().size() == 10);
    REQUIRE(other.getArrPtr().size() == 10);
    REQUIRE(other.getArrSimple().size() == 10);

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(other.getEmptyVec().empty());
    for (int i = 0; i < 10; ++i) {
        REQUIRE(original.getVec()[i] == other.getVec()[i]);
        REQUIRE(original.getClassVec()[i] == other.getClassVec()[i]);
        REQUIRE(original.getArr()[i] == other.getArr()[i]);
        if (i % 2) {
            REQUIRE(original.getArrPtr()[i] == other.getArrPtr()[i]);
        } else {
            REQUIRE(other.getArrPtr()[i] != nullptr);
            REQUIRE(*original.getArrPtr()[i] == *other.getArrPtr()[i]);
        }
        REQUIRE(original.getArrSimple()[i] == other.getArrSimple()[i]);
        for (int j = 0; j < 5; ++j) {
            REQUIRE(original.getVec2D().at(i).at(j) ==
                    original.getVec2D().at(i).at(j));
        }
    }

    auto it = original.getLst().begin();
    for (double d : other.getLst()) {
        REQUIRE(d == *it++);
    }

    REQUIRE(original.getArr().data() != other.getArr().data());
    REQUIRE(original.getVec().data() != other.getVec().data());
    REQUIRE(original.getVec2D().data() != other.getVec2D().data());
    REQUIRE(original.getClassVec().data() != other.getClassVec().data());
    REQUIRE(original.getLst().begin() != other.getLst().begin());

    for (size_t i = 0; i < 10; ++i) {
        delete original.getArrPtr()[i];
        delete other.getArrPtr()[i];
    }
}
#endif

/******************************************************************************/
/*                       implementation of a serializer                       */
/******************************************************************************/

#ifdef TEST_POLYMORPHIC
#include "test-classes/abstract.hpp"
TEST_CASE("implementing a serializer (polymorphic class serialization)") {
    AbstractCollection original;
    AbstractCollection other;
    serializer::default_mem_type result;

    original.push_back(new Concrete1(1, 2.9));
    original.push_back(new Concrete2("hello \"test\" world"));

    original.push_back(std::make_shared<Concrete1>(2, 3.9));
    original.push_back(std::make_shared<Concrete2>("pif \"paf\" pouf"));

    original.add_unique(std::make_unique<Concrete1>(4, 5.9));
    original.add_unique(std::make_unique<Concrete2>("a \"b\" c"));

    REQUIRE(original.elements().size() == 2);
    REQUIRE(other.elements().empty());

    original.serialize(result);
    other.deserialize(result);

    // pointer vector
    REQUIRE(other.elements().size() == original.elements().size());
    auto itPtr = other.elements().begin();
    for (auto *sa : original.elements()) {
        REQUIRE(*sa == *itPtr++);
    }

    // shared pointer vector
    REQUIRE(other.elementsShared().size() == original.elementsShared().size());
    auto itShared = other.elementsShared().begin();
    for (auto &sa : original.elementsShared()) {
        REQUIRE(*sa == *itShared++);
    }

    // unique pointer vector
    REQUIRE(other.elementsUnique().size() == original.elementsUnique().size());
    auto itUnique = other.elementsUnique().begin();
    for (auto const &sa : original.elementsUnique()) {
        REQUIRE(*sa == *itUnique++);
    }
}
#endif

/******************************************************************************/
/*                                inheritance                                 */
/******************************************************************************/

#ifdef TEST_SUPER
#include "test-classes/polymorphic.hpp"
TEST_CASE("serialize super class") {
    SuperCollection original;
    SuperCollection other;
    serializer::default_mem_type result;
    auto *c1 = new Class1("John", 20, 1, 2.9);
    auto *c2 = new Class2("David", 30, "hello world");

    // test with super class serialization:

    c1->serialize(result);
    Class1 c11;
    c11.deserialize(result);
    REQUIRE(c11 == c1);

    c2->serialize(result); // test the second method (faster)
    Class2 c22;
    c22.deserialize(result);
    REQUIRE(c22 == c2);

    original.push_back(c1);
    original.push_back(c2);

    REQUIRE(original.getElements().size() == 2);
    REQUIRE(other.getElements().empty());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(other.getElements().size() == original.getElements().size());
    auto it = other.getElements().begin();
    for (SuperClass *sa : original.getElements()) {
        REQUIRE(*sa == *it++);
    }
}
#endif

/******************************************************************************/
/*                            multiple inheritance                            */
/******************************************************************************/

#ifdef TEST_INHERITANCE
#include "test-classes/multipleinheritance.hpp"
TEST_CASE("multiple inheritance") {
    mi::Collection original;
    mi::Collection other;
    serializer::default_mem_type result;
    auto *c1 = new mi::Daughter1(10, "test1", 2.2);
    auto *c2 = new mi::Daughter2(10, "test2", 2.2, "job");
    auto *c3 = new mi::Daughter2(10, "test3", 2.2, "other job");

    // test with super class serialization:

    c1->serialize(result);
    mi::Daughter1 c11;
    c11.deserialize(result);
    REQUIRE(c11.operator==(c1));

    c2->serialize(result);
    mi::Daughter2 c22;
    c22.deserialize(result);
    REQUIRE(c22.operator==(c2));

    original.push_back(c1);
    original.push_back(c2);
    original.push_back(c3);

    REQUIRE(original.elements().size() == 3);
    REQUIRE(other.elements().empty());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(other.elements().size() == original.elements().size());
    auto it = other.elements().begin();
    for (mi::Mother *m : original.elements()) {
        REQUIRE(*m == *it++);
    }

    // delete
    for (mi::Mother *m : other.elements()) {
        delete m;
    }
    delete c1;
    delete c2;
    delete c3;
}
#endif

/******************************************************************************/
/*                               smart pointers                               */
/******************************************************************************/

#ifdef TEST_SMART_PTR
#include "test-classes/withsmartptr.hpp"
TEST_CASE("smart pointers") {
    WithSmartPtr original(1, 2.3, "hello");
    WithSmartPtr other;
    serializer::default_mem_type result;

    REQUIRE(original.intPtr() != other.intPtr());
    REQUIRE(original.doublePtr() != other.doublePtr());
    REQUIRE(original.otherType() != other.otherType());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.intPtr() == other.intPtr());
    REQUIRE(original.doublePtr() == other.doublePtr());
    REQUIRE(original.otherType() == other.otherType());
}
#endif

/******************************************************************************/
/*                                   enums                                    */
/******************************************************************************/

#ifdef TEST_ENUMS
#include "test-classes/withenums.hpp"
TEST_CASE("enums") {
    WithEnums original("str_", SUNDAY, VISUAL, DndClasses::ROGUE);
    WithEnums other;
    serializer::default_mem_type result;

    REQUIRE(original.getNormalType() != other.getNormalType());
    REQUIRE(original.getDay() != other.getDay());
    REQUIRE(original.getMode() != other.getMode());
    REQUIRE(original.getClass() != other.getClass());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.getNormalType() == other.getNormalType());
    REQUIRE(original.getDay() == other.getDay());
    REQUIRE(original.getMode() == other.getMode());
    REQUIRE(original.getClass() == other.getClass());
}
#endif

/******************************************************************************/
/*                                   pairs                                    */
/******************************************************************************/

#ifdef TEST_PAIR
#include "test-classes/withpair.hpp"
TEST_CASE("pairs") {
    std::vector<int> v = {1, 2, 3, 4};
    std::set<std::string> s = {"hello", "world"};
    WithPair original(1, 2, "hello", "world", Simple(10, 20),
                      Composed(Simple(10, 20), 3, 3.14), v, s);
    WithPair other;
    serializer::default_mem_type result;

    REQUIRE(original.intPair().first != other.intPair().first);
    REQUIRE(original.intPair().second != other.intPair().second);
    REQUIRE(original.stringPair().first != other.stringPair().first);
    REQUIRE(original.stringPair().first != other.stringPair().first);
    REQUIRE(original.objPair().second != other.objPair().second);
    REQUIRE(original.objPair().second != other.objPair().second);
    REQUIRE(original.getContainerPair().first.size() !=
            other.getContainerPair().first.size());
    REQUIRE(original.getContainerPair().second.size() !=
            other.getContainerPair().second.size());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(original.intPair().first == other.intPair().first);
    REQUIRE(original.intPair().second == other.intPair().second);
    REQUIRE(original.stringPair().first == other.stringPair().first);
    REQUIRE(original.stringPair().second == other.stringPair().second);
    REQUIRE(original.objPair().second == other.objPair().second);
    REQUIRE(original.objPair().second == other.objPair().second);
    REQUIRE(original.getContainerPair().first.size() ==
            other.getContainerPair().first.size());
    REQUIRE(original.getContainerPair().second.size() ==
            other.getContainerPair().second.size());

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
#endif

/******************************************************************************/
/*                                   tuples                                   */
/******************************************************************************/

#ifdef TEST_TUPLE
#include "test-classes/withtuple.hpp"
TEST_CASE("tuples") {
    std::vector<int> v = {1, 2, 3, 4};
    std::set<std::string> s = {"hello", "world"};
    std::map<std::string, std::string> m = {{"foo", "barr"},
                                            {"hello", "world"}};
    WithTuple original(1, 2, 1.618, "hello", "world", "!", Simple(10, 20),
                       Composed(Simple(10, 20), 3, 3.14), v, s, m);
    WithTuple other;
    serializer::default_mem_type result;

    REQUIRE(std::get<0>(original.numberTuple()) !=
            std::get<0>(other.numberTuple()));
    REQUIRE(std::get<1>(original.numberTuple()) !=
            std::get<1>(other.numberTuple()));
    REQUIRE(std::get<2>(original.numberTuple()) !=
            std::get<2>(other.numberTuple()));

    REQUIRE(std::get<0>(original.stringTuple()) !=
            std::get<0>(other.stringTuple()));
    REQUIRE(std::get<1>(original.stringTuple()) !=
            std::get<1>(other.stringTuple()));
    REQUIRE(std::get<2>(original.stringTuple()) !=
            std::get<2>(other.stringTuple()));

    REQUIRE(std::get<0>(original.objTuple()) != std::get<0>(other.objTuple()));
    REQUIRE(std::get<1>(original.objTuple()) != std::get<1>(other.objTuple()));

    REQUIRE(std::get<0>(original.containerTuple()).size() !=
            std::get<0>(other.containerTuple()).size());
    REQUIRE(std::get<1>(original.containerTuple()).size() !=
            std::get<1>(other.containerTuple()).size());
    REQUIRE(std::get<2>(original.containerTuple()).size() !=
            std::get<2>(other.containerTuple()).size());

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(std::get<0>(original.numberTuple()) ==
            std::get<0>(other.numberTuple()));
    REQUIRE(std::get<1>(original.numberTuple()) ==
            std::get<1>(other.numberTuple()));
    REQUIRE(std::get<2>(original.numberTuple()) ==
            std::get<2>(other.numberTuple()));

    REQUIRE(std::get<0>(original.stringTuple()) ==
            std::get<0>(other.stringTuple()));
    REQUIRE(std::get<1>(original.stringTuple()) ==
            std::get<1>(other.stringTuple()));
    REQUIRE(std::get<2>(original.stringTuple()) ==
            std::get<2>(other.stringTuple()));

    REQUIRE(std::get<0>(original.objTuple()) == std::get<0>(other.objTuple()));
    REQUIRE(std::get<1>(original.objTuple()) == std::get<1>(other.objTuple()));

    REQUIRE(std::get<0>(original.containerTuple()).size() ==
            std::get<0>(other.containerTuple()).size());
    REQUIRE(std::get<1>(original.containerTuple()).size() ==
            std::get<1>(other.containerTuple()).size());
    REQUIRE(std::get<2>(original.containerTuple()).size() ==
            std::get<2>(other.containerTuple()).size());

    auto originalFirstIt = std::get<0>(original.containerTuple()).begin();
    auto otherFirstIt = std::get<0>(other.containerTuple()).begin();
    while (originalFirstIt != std::get<0>(original.containerTuple()).end()) {
        REQUIRE(*originalFirstIt++ == *otherFirstIt++);
    }

    auto originalSecondIt = std::get<1>(original.containerTuple()).begin();
    auto otherSecondIt = std::get<1>(other.containerTuple()).begin();
    while (originalSecondIt != std::get<1>(original.containerTuple()).end()) {
        REQUIRE(*originalSecondIt++ == *otherSecondIt++);
    }

    auto originalThirdIt = std::get<2>(original.containerTuple()).begin();
    auto otherThirdIt = std::get<2>(other.containerTuple()).begin();
    while (originalThirdIt != std::get<2>(original.containerTuple()).end()) {
        REQUIRE(*originalThirdIt++ == *otherThirdIt++);
    }
}
#endif

/******************************************************************************/
/*                                 serializer                                 */
/******************************************************************************/

#ifdef TEST_SERIALIZER
#include "test-classes/withserializer.hpp"
TEST_CASE("serialize unknown type") {
    WithSerializer original;
    WithSerializer other;
    serializer::default_mem_type result;

    original.addInt(1);
    original.addInt(2);
    original.addInt(3);
    original.addUnknown(Unknown(1));
    original.addUnknown(Unknown(2));
    original.addUnknown(Unknown(3));

    REQUIRE(!original.getInts().empty());
    REQUIRE(!original.getUnknowns().empty());
    REQUIRE(other.getInts().empty());
    REQUIRE(other.getUnknowns().empty());

    original.serialize(result);
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
#endif

/******************************************************************************/
/*                                    map                                     */
/******************************************************************************/

#ifdef TEST_MAP
#include "test-classes/withmap.hpp"
TEST_CASE("map") {
    WithMap original;
    WithMap other;
    serializer::default_mem_type result;

    REQUIRE(original.map().empty());
    REQUIRE(other.map().empty());

    original.insert("hello", "pouf");
    original.insert("world", "pouf");

    REQUIRE(original.map().size() == 2);

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(other.map().size() == original.map().size());
    REQUIRE(other.map().at("hello") == "pouf");
    REQUIRE(other.map().at("world") == "pouf");
}
#endif

/******************************************************************************/
/*                                    set                                     */
/******************************************************************************/

#ifdef TEST_SET
#include "test-classes/withset.hpp"
TEST_CASE("set") {
    WithSet original;
    WithSet other;
    serializer::default_mem_type result;

    REQUIRE(original.set().empty());
    REQUIRE(other.set().empty());

    original.insert("hello");
    original.insert("world");

    REQUIRE(original.set().size() == 2);

    original.serialize(result);
    other.deserialize(result);

    REQUIRE(other.set().size() == original.set().size());
    auto original_it = original.set().begin();
    for (const auto &other_it : other.set()) {
        REQUIRE(other_it == *original_it);
        original_it++;
    }
}
#endif

/******************************************************************************/
/*                                  cstruct                                   */
/******************************************************************************/

#ifdef TEST_CSTRUCT
#include "test-classes/cstruct.h"
#define time(t)                                                                \
    std::chrono::duration_cast<std::chrono::nanoseconds>(t).count() << "ns"

TEST_CASE("cstruct") {
    CStruct cs = {.c = 'c', .i = 4, .l = 12347890, .f = 3.14, .d = 1.618};
    CStruct otherCS = {};
    CStructSerializable css('c', 4, 12347890, 3.14, 1.618);
    CStructSerializable otherCSS;
    serializer::default_mem_type result(40);
    serializer::default_mem_type resultSerializable(40);
    std::string name;
    std::string nameDeserialization;

    // c like serialization
    auto begin = std::chrono::system_clock::now();
    result.append(0, reinterpret_cast<uint8_t *>(&cs), sizeof(cs));
    /* if constexpr (serializer::concepts::Resizeable<decltype(result)>) { */
    /*     result.resize(sizeof(cs)); */
    /* } */
    /* std::memcpy(result.data(), reinterpret_cast<uint8_t *>(&cs), */
    /*             sizeof(cs)); */
    auto end = std::chrono::system_clock::now();
    std::cout << "c serialization time: " << time(end - begin) << std::endl;

    // serializer serialization
    begin = std::chrono::system_clock::now();
    css.serialize(resultSerializable);
    end = std::chrono::system_clock::now();
    std::cout << "serializer serialization time: " << time(end - begin)
              << std::endl;

    // c like deserialization
    begin = std::chrono::system_clock::now();
    otherCS = *reinterpret_cast<CStruct *>(result.data());
    end = std::chrono::system_clock::now();
    std::cout << "c deserialization time: " << time(end - begin) << std::endl;

    // serializer deserialization
    begin = std::chrono::system_clock::now();
    otherCSS.deserialize(resultSerializable);
    end = std::chrono::system_clock::now();
    std::cout << "serializer deserialization time: " << time(end - begin)
              << std::endl;

    // test cs deserialization
    REQUIRE(otherCS.c == cs.c);
    REQUIRE(otherCS.i == cs.i);
    REQUIRE(otherCS.l == cs.l);
    REQUIRE(otherCS.f == cs.f);
    REQUIRE(otherCS.d == cs.d);
    REQUIRE(nameDeserialization == name);

    // test css deserialization
    REQUIRE(otherCSS.c() == css.c());
    REQUIRE(otherCSS.i() == css.i());
    REQUIRE(otherCSS.l() == css.l());
    REQUIRE(otherCSS.f() == css.f());
    REQUIRE(otherCSS.d() == css.d());
}
#endif

/******************************************************************************/
/*                                  function                                  */
/******************************************************************************/

#ifdef TEST_FUNCTION
#include "test-classes/withfunctions.hpp"
TEST_CASE("functions") {
    WithFunctions origin(4);
    WithFunctions other;
    serializer::default_mem_type result;

    REQUIRE(origin.i() != other.i());

    origin.serialize(result);
    other.deserialize(result);

    REQUIRE(other.i() == origin.i());
}
#endif

/******************************************************************************/
/*                               static arrays                                */
/******************************************************************************/

#ifdef TEST_STATIC_ARRAYS
#include "test-classes/withstaticarrays.hpp"
TEST_CASE("static arrays") {
    WithStaticArrays origin;
    WithStaticArrays other;
    serializer::default_mem_type result;

    // filling the arrays
    for (size_t i = 0; i < 10; ++i) {
        origin.arr(i) = i;
        for (size_t j = 0; j < 10; ++j) {
            origin.grid(i, j) = i + j;
            for (size_t k = 0; k < 2; ++k) {
                origin.tensor(i, j, k) = i + j + k;
            }
        }
    }

    for (size_t i = 0; i < 2; ++i) {
        origin.arrSimple(i) = Simple(i, i, "hello");
        for (size_t j = 0; j < 2; ++j) {
            origin.gridSimple(i, j) = Simple(i, j, "world");
        }
    }

    origin.serialize(result);
    other.deserialize(result);

    for (size_t i = 0; i < 10; ++i) {
        REQUIRE(origin.arr(i) == other.arr(i));
        for (size_t j = 0; j < 10; ++j) {
            REQUIRE(origin.grid(i, j) == other.grid(i, j));
            for (size_t k = 0; k < 2; ++k) {
                REQUIRE(origin.tensor(i, j, k) == other.tensor(i, j, k));
            }
        }
    }

    for (size_t i = 0; i < 2; ++i) {
        REQUIRE(origin.arrSimple(i) == other.arrSimple(i));
        for (size_t j = 0; j < 2; ++j) {
            REQUIRE(origin.gridSimple(i, j) == other.gridSimple(i, j));
        }
    }
}
#endif

/******************************************************************************/
/*                               dynamic arrays                               */
/******************************************************************************/

#ifdef TEST_DYNAMIC_ARRAYS
#include "test-classes/withdynamicarrays.hpp"
TEST_CASE("dynamic arrays") {
    WithDynamicArray origin(2);
    WithDynamicArray other;
    auto *external = new double[10];
    serializer::default_mem_type result;

    for (size_t i = 0; i < 10; ++i) {
        external[i] = (double)i * 3.0;
    }
    origin.borrow(external, 10);

    for (size_t i = 0; i < (4 * 4); ++i) {
        origin.twoDOneD()[i] = (int)i * 2;
    }

    for (size_t i = 0; i < 5; ++i) {
        origin.own()[i] = (int)i;
        origin.ownSimple()[i] = Simple((int)i, (int)i, "simple");
    }

    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            origin.multipleDim()[i][j] = (int)(i + j);
        }
    }

    origin.serialize(result);
    other.deserialize(result);

    for (size_t i = 0; i < 10; ++i) {
        REQUIRE(external[i] == other.borrowed()[i]);
    }

    for (size_t i = 0; i < (4 * 4); ++i) {
        REQUIRE(origin.twoDOneD()[i] == other.twoDOneD()[i]);
    }

    for (size_t i = 0; i < 5; ++i) {
        REQUIRE(origin.own()[i] == other.own()[i]);
        REQUIRE(origin.ownSimple()[i] == other.ownSimple()[i]);
        REQUIRE(origin.null2()[i] == other.null2()[i]);
    }
    REQUIRE(origin.null() == other.null());

    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            REQUIRE(origin.multipleDim()[i][j] == other.multipleDim()[i][j]);
        }
    }

    delete[] external;
    delete[] other.borrowed();
}
#endif

/******************************************************************************/
/*                                    tree                                    */
/******************************************************************************/

#ifdef TEST_TREE
#include "test-classes/tree.hpp"
#include <stack>
TEST_CASE("tree") {
    Tree<int> origin;
    Tree<int> other;
    serializer::default_mem_type result;

    for (auto elt : {5, 4, 8, 6, 1, 3, 7, 2, 9}) {
        origin.insert(elt);
    }

    origin.serialize(result);
    other.deserialize(result);

    Node<int> **currOrigin = &origin.root;
    Node<int> **currOther = &other.root;
    std::stack<Node<int> **> nodes;

    while (*currOrigin && *currOther) {
        // verify the value
        REQUIRE((*currOther)->value == (*currOrigin)->value);
        // verify the right son father & push
        if ((*currOrigin)->right) {
            REQUIRE((*currOther)->right);
            REQUIRE((*currOther)->right->father == *currOther);
            nodes.push(&(*currOrigin)->right);
            nodes.push(&(*currOther)->right);
        }
        // verify the left son father
        if ((*currOrigin)->left) {
            REQUIRE((*currOther)->left);
            REQUIRE((*currOther)->left->father == *currOther);
        }
        // move to the left
        currOrigin = &(*currOrigin)->left;
        currOther = &(*currOther)->left;
        // pop the stack if leaf
        if (!*currOrigin && !nodes.empty()) {
            currOther = nodes.top();
            nodes.pop();
            currOrigin = nodes.top();
            nodes.pop();
        }
    }
}
#endif

/******************************************************************************/
/*                                  hedgehog                                  */
/******************************************************************************/

#ifdef TEST_HH
#include "test-classes/hedgehog.hpp"
TEST_CASE("hedgehog") {
    constexpr size_t w = 4, h = 4, bs = 2;
    double sum = 0;
    double *data = new double[h * w];
    serializer::default_mem_type buff;
    auto matrix = std::make_shared<Matrix<double>>(h, w, bs, data);

    // Tasks
    auto st = std::make_shared<SplitTask<double>>();
    auto ct = std::make_shared<ComputeTask<double>>();
    auto rt = std::make_shared<ResultTask<double>>();

    TaskManager<TypeTable<double>, SplitTask<double>, ComputeTask<double>,
                ResultTask<double>>
        tm(st, ct, rt);

    // setup the matrix
    for (size_t i = 0; i < 16; ++i) {
        matrix->data()[i] = i;
        sum += (double)i;
    }
    matrix->serialize(buff);
    Network::send(buff);

    tm.receive(Network::rcv()); // receive the matrix in the split task
    tm.receive(Network::rcv()); // receive the blocks in the compute task
    tm.receive(Network::rcv()); // receive the partial sums in the result task

    REQUIRE(rt->result == sum);

    delete[] data;
}
#endif
