#include "catch.hpp"
#include "test-classes/abstract.hpp"
#include "test-classes/composed.hpp"
#include "test-classes/cstruct.h"
#include "test-classes/multipleinheritance.hpp"
#include "test-classes/polymorphic.hpp"
#include "test-classes/simple.hpp"
#include "test-classes/tree.hpp"
#include "test-classes/withMap.hpp"
#include "test-classes/withPair.hpp"
#include "test-classes/withSmartPtr.hpp"
#include "test-classes/withcontainer.hpp"
#include "test-classes/withconvertor.hpp"
#include "test-classes/withdynamicarrays.hpp"
#include "test-classes/withenums.hpp"
#include "test-classes/withfunctions.hpp"
#include "test-classes/withpointers.hpp"
#include "test-classes/withset.hpp"
#include "test-classes/withstaticarrays.hpp"
#include "test-classes/withstring.hpp"
#include "test-classes/withtuple.hpp"
#include <chrono>
#include <stack>
#include <string>

/******************************************************************************/
/*                         tests with a simple class                          */
/******************************************************************************/

TEST_CASE("serialization/deserialization on a SIMPLE CLASS") {
    Simple original(10, 20, "hello \"world!\\");
    Simple other(0, 0);
    std::string result;

    REQUIRE(original.x() != other.x());
    REQUIRE(original.y() != other.y());
    REQUIRE(original.str() != other.str());

    result = original.serialize();
    other.deserialize(result);

    // the serialization and deserialization work
    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());

    original.x(3); // modify an attribute

    REQUIRE(original.x() != other.x());

    result = original.serialize();
    other.deserialize(result);

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.x() == other.x());

    original.x(28);
    original.y(32);
    Simple copied = original;

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());
}

/******************************************************************************/
/*          tests with a class composed of a serializable subclass            */
/******************************************************************************/

TEST_CASE("serialization/deserialization on a COMPOSED CLASS") {
    Composed original(Simple(10, 20), 3, 3.14);
    Composed other(Simple(0, 0), 0, 0);
    std::string result;

    REQUIRE(original.s().x() != other.s().x());
    REQUIRE(original.s().y() != other.s().y());
    REQUIRE(original.z() != other.z());
    REQUIRE(original.w() != other.w());

    result = original.serialize();
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

    result = original.serialize();
    other.deserialize(result);

    // again, we check if the references allow tracking of modifications
    REQUIRE(original.s().x() == other.s().x());
    REQUIRE(original.s().y() == other.s().y());
    REQUIRE(original.z() == other.z());
    REQUIRE(original.w() == other.w());
}

/******************************************************************************/
/*                            string serialization                            */
/******************************************************************************/

TEST_CASE("serialization/deserialization with STRING ATTRIBUTE") {
    WithString original(2, "hello");
    WithString originalEmptyString(3, "");
    WithString other(0, "world");
    std::string result;

    REQUIRE(original.x() != other.x());
    REQUIRE(original.str() != other.str());
    REQUIRE(originalEmptyString.x() != other.x());
    REQUIRE(originalEmptyString.str() != other.str());

    // with non-empty string
    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.x() == other.x());
    REQUIRE(original.str() == other.str());

    // with empty string
    result = originalEmptyString.serialize();
    other.deserialize(result);

    REQUIRE(originalEmptyString.x() == other.x());
    REQUIRE(originalEmptyString.str() == other.str());

    originalEmptyString.str("world");

    result = originalEmptyString.serialize();
    other.deserialize(result);

    REQUIRE(originalEmptyString.x() == other.x());
    REQUIRE(originalEmptyString.str() == other.str());
}

/******************************************************************************/
/*                           pointer serialization                            */
/******************************************************************************/

/* IMPORTANT: this test should be run with valgrind. */
TEST_CASE("serialization/deserialization with POINTERS ATTRIBUTE") {
    WithPointers original(new Simple(1, 2));
    WithPointers other(new Simple(0, 0));
    std::string result;

    REQUIRE(original.classPointer()->x() != other.classPointer()->x());
    REQUIRE(original.classPointer()->y() != other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() ==
            *other.fundamentalPointer()); // this one doesn't move here

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.classPointer()->x() == other.classPointer()->x());
    REQUIRE(original.classPointer()->y() == other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() == *other.fundamentalPointer());

    original.classPointer(new Simple(20, 30));
    original.fundamentalPointer(new double(3.14));

    REQUIRE(original.classPointer()->x() != other.classPointer()->x());
    REQUIRE(original.classPointer()->y() != other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() != *other.fundamentalPointer());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.classPointer()->x() == other.classPointer()->x());
    REQUIRE(original.classPointer()->y() == other.classPointer()->y());
    REQUIRE(*original.fundamentalPointer() == *other.fundamentalPointer());
}

/******************************************************************************/
/*                  serialization with a members_ attribute                  */
/******************************************************************************/

/*
 * NOTE: there is a problem with precision for doubles.
 */
TEST_CASE("serialization/deserialization with ITERABLES ATTRIBUTE") {
    WithContainer original;
    WithContainer other;
    std::string result;

    // adding elements_ in containers
    for (int i = 0; i < 10; ++i) {
        original.addInt(i);
        original.addDouble(double(i));
        original.addSimple(Simple(i, 2 * i));
        original.addVec(std::vector<int>{1 * i, 2 * i, 3 * i, 4 * i, 5 * i});
        original.addArr(i, i * 2);
    }

    REQUIRE(original.getEmptyVec().empty());
    REQUIRE(other.getEmptyVec().empty());
    REQUIRE(original.getVec().size() == 10);
    REQUIRE(original.getLst().size() == 10);
    REQUIRE(original.getClassVec().size() == 10);
    REQUIRE(original.getArr().size() == 10);
    REQUIRE(other.getVec().empty());
    REQUIRE(other.getLst().empty());
    REQUIRE(other.getClassVec().empty());
    REQUIRE(other.getVec2D().empty());
    REQUIRE(other.getArr().size() == 10);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.getEmptyVec().empty());
    for (int i = 0; i < 10; ++i) {
        REQUIRE(original.getVec()[i] == other.getVec()[i]);
        REQUIRE(original.getClassVec()[i] == other.getClassVec()[i]);
        REQUIRE(original.getArr()[i] == other.getArr()[i]);
        for (int j = 0; j < 5; ++j) {
            REQUIRE(original.getVec2D().at(i).at(j) ==
                    original.getVec2D().at(i).at(j));
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

    original.push_back(std::make_shared<Concrete1>(2, 3.9));
    original.push_back(std::make_shared<Concrete2>("pif \"paf\" pouf"));

    original.add_unique(std::make_unique<Concrete1>(4, 5.9));
    original.add_unique(std::make_unique<Concrete2>("a \"b\" c"));

    REQUIRE(original.elements().size() == 2);
    REQUIRE(other.elements().empty());

    result = original.serialize();
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

/******************************************************************************/
/*                                inheritance                                 */
/******************************************************************************/

TEST_CASE("serialize super class") {
    SuperCollection original;
    SuperCollection other;
    std::string result;
    auto *c1 = new Class1("John", 20, 1, 2.9);
    auto *c2 = new Class2("David", 30, "hello world");

    // test with super class serialization:

    result = c1->serialize();
    Class1 c11;
    c11.deserialize(result);
    REQUIRE(c11.serialize() == c1->serialize());

    result.clear();
    c2->serialize(result); // test the second method (faster)
    Class2 c22;
    c22.deserialize(result);
    REQUIRE(c22.serialize() == c2->serialize());

    original.push_back(c1);
    original.push_back(c2);

    REQUIRE(original.getElements().size() == 2);
    REQUIRE(other.getElements().empty());

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

    REQUIRE(original.x() != other.x());
    REQUIRE(original.y() != other.y());

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    // the serialization and deserialization work
    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());

    original.x(3); // modify an attribute

    REQUIRE(original.x() != other.x());

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    // the modification is taken in count by the serializer (references)
    REQUIRE(original.x() == other.x());

    original.x(28);
    original.y(32);
    Simple copied = original;

    original.serializeFile("test_serialize.txt");
    other.deserializeFile("test_serialize.txt");

    REQUIRE(original.x() == other.x());
    REQUIRE(original.y() == other.y());
}

/******************************************************************************/
/*                            multiple inheritance                            */
/******************************************************************************/

TEST_CASE("multiple inheritance") {
    mi::Collection original;
    mi::Collection other;
    std::string result;
    auto *c1 = new mi::Daughter1(10, "test1", 2.2);
    auto *c2 = new mi::Daughter2(10, "test2", 2.2, "job");
    auto *c3 = new mi::Daughter2(10, "test3", 2.2, "other job");

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

    REQUIRE(original.elements().size() == 3);
    REQUIRE(other.elements().empty());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.elements().size() == original.elements().size());
    auto it = other.elements().begin();
    for (mi::Mother *m : original.elements()) {
        REQUIRE(*m == *it++);
    }
}

/******************************************************************************/
/*                               smart pointers                               */
/******************************************************************************/

TEST_CASE("smart pointers") {
    WithSmartPtr original(1, 2.3, "hello");
    WithSmartPtr other;
    std::string result;

    REQUIRE(original.intPtr() != other.intPtr());
    REQUIRE(original.doublePtr() != other.doublePtr());
    REQUIRE(original.otherType() != other.otherType());

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(original.intPtr() == other.intPtr());
    REQUIRE(original.doublePtr() == other.doublePtr());
    REQUIRE(original.otherType() == other.otherType());
}

/******************************************************************************/
/*                                   enums                                    */
/******************************************************************************/

TEST_CASE("enums") {
    WithEnums original("str_", SUNDAY, VISUAL, DndClasses::ROGUE);
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
    std::vector<int> v = {1, 2, 3, 4};
    std::set<std::string> s = {"hello", "world"};
    WithPair original(1, 2, "hello", "world", Simple(10, 20),
                      Composed(Simple(10, 20), 3, 3.14), v, s);
    WithPair other;
    std::string result;

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

    result = original.serialize();
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

/******************************************************************************/
/*                                   tuples                                   */
/******************************************************************************/

TEST_CASE("tuples") {
    std::vector<int> v = {1, 2, 3, 4};
    std::set<std::string> s = {"hello", "world"};
    std::map<std::string, std::string> m = {{"foo", "barr"},
                                            {"hello", "world"}};
    WithTuple original(1, 2, 1.618, "hello", "world", "!", Simple(10, 20),
                       Composed(Simple(10, 20), 3, 3.14), v, s, m);
    WithTuple other;
    std::string result;

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

    result = original.serialize();
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

    REQUIRE(!original.getInts().empty());
    REQUIRE(!original.getUnknowns().empty());
    REQUIRE(other.getInts().empty());
    REQUIRE(other.getUnknowns().empty());

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

    REQUIRE(original.map().empty());
    REQUIRE(other.map().empty());

    original.insert("hello", "pouf");
    original.insert("world", "pouf");

    REQUIRE(original.map().size() == 2);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.map().size() == original.map().size());
    REQUIRE(other.map().at("hello") == "pouf");
    REQUIRE(other.map().at("world") == "pouf");
}

/******************************************************************************/
/*                                    set                                     */
/******************************************************************************/

TEST_CASE("set") {
    WithSet original;
    WithSet other;
    std::string result;

    REQUIRE(original.set().empty());
    REQUIRE(other.set().empty());

    original.insert("hello");
    original.insert("world");

    REQUIRE(original.set().size() == 2);

    result = original.serialize();
    other.deserialize(result);

    REQUIRE(other.set().size() == original.set().size());
    auto original_it = original.set().begin();
    for (const auto &other_it : other.set()) {
        REQUIRE(other_it == *original_it);
        original_it++;
    }
}

/******************************************************************************/
/*                                  cstruct                                   */
/******************************************************************************/

TEST_CASE("cstruct") {
    CStruct cs = {.c = 'c', .i = 4, .l = 12347890, .f = 3.14, .d = 1.618};
    CStruct otherCS = {};
    CStructSerializable css('c', 4, 12347890, 3.14, 1.618);
    CStructSerializable otherCSS;
    std::string result;
    std::string resultSerializable;
    std::string name;
    std::string nameDeserialization;
    size_t size;

    // c like serialization
    auto begin = std::chrono::system_clock::now();
    name = typeid(cs).name();
    size = name.size();
    result.append(reinterpret_cast<char *>(&size), sizeof(size));
    result.append(name);
    result.append(reinterpret_cast<char *>(&cs), sizeof(cs));
    auto end = std::chrono::system_clock::now();
    std::cout << "c serialization time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      begin)
                     .count()
              << "ns" << std::endl;

    // serializer serialization
    begin = std::chrono::system_clock::now();
    css.serialize(resultSerializable);
    end = std::chrono::system_clock::now();
    std::cout << "serializer serialization time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      begin)
                     .count()
              << "ns" << std::endl;

    // c like deserialization
    begin = std::chrono::system_clock::now();
    size = *reinterpret_cast<size_t *>(result.data());
    nameDeserialization.append(
        reinterpret_cast<char *>(result.data() + sizeof(size)), size);
    otherCS = *reinterpret_cast<CStruct *>(result.data() + sizeof(size) + size);
    end = std::chrono::system_clock::now();
    std::cout << "c deserialization time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      begin)
                     .count()
              << "ns" << std::endl;

    // serializer deserialization
    begin = std::chrono::system_clock::now();
    otherCSS.deserialize(resultSerializable);
    end = std::chrono::system_clock::now();
    std::cout << "serializer deserialization time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                      begin)
                     .count()
              << "ns" << std::endl;

    // test cs deserialization
    REQUIRE(otherCS.c == cs.c);
    REQUIRE(otherCS.i == cs.i);
    REQUIRE(otherCS.l == cs.l);
    REQUIRE(otherCS.f == cs.f);
    REQUIRE(otherCS.d == cs.d);
    REQUIRE(nameDeserialization == name);

    // test css deserialization
    REQUIRE(otherCSS.c == css.c);
    REQUIRE(otherCSS.i == css.i);
    REQUIRE(otherCSS.l == css.l);
    REQUIRE(otherCSS.f == css.f);
    REQUIRE(otherCSS.d == css.d);
}

/******************************************************************************/
/*                                  function                                  */
/******************************************************************************/

TEST_CASE("functions") {
    int originI = 4;
    WithFunctions origin(originI);
    WithFunctions other;
    std::string result;

    REQUIRE(origin.i() != other.i());

    result = origin.serialize();
    other.deserialize(result);

    REQUIRE(origin.i() == originI * 2);
    REQUIRE(other.i() == originI * originI);
}

/******************************************************************************/
/*                               static arrays                                */
/******************************************************************************/

TEST_CASE("static arrays") {
    WithStaticArrays origin;
    WithStaticArrays other;
    std::string result;

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

    result = origin.serialize();
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

/******************************************************************************/
/*                               dynamic arrays                               */
/******************************************************************************/

TEST_CASE("dynamic arrays") {
    WithDynamicArray origin(2);
    WithDynamicArray other;
    auto *external = new double[10];
    std::string result;

    for (size_t i = 0; i < 10; ++i) {
        external[i] = (double) i * 3.0;
    }
    origin.borrow(external, 10);

    for (size_t i = 0; i < (4 * 4); ++i) {
        origin.twoDOneD()[i] = (int) i * 2;
    }

    for (size_t i = 0; i < 5; ++i) {
        origin.own()[i] = (int) i;
        origin.ownSimple()[i] = Simple((int) i, (int) i, "simple");
    }

    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            origin.multipleDim()[i][j] = (int) (i + j);
        }
    }

    result = origin.serialize();
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

/******************************************************************************/
/*                                    tree                                    */
/******************************************************************************/

TEST_CASE("tree") {
  Tree<int> origin;
  Tree<int> other;
  std::string result;

  for (auto elt : {5, 4, 8, 6, 1, 3, 7, 2, 9}) {
    origin.insert(elt);
  }
  result = origin.serialize();
  other.deserialize(result);

  Node<int> **currOrigin = &origin.root;
  Node<int> **currOther = &other.root;
  std::stack<Node<int>**> nodes;

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
