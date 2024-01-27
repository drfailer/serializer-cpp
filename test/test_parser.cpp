#include "catch.hpp"
#include "serializer/parser.hpp"
#include <string>

/******************************************************************************/
/*                               test variables                               */
/******************************************************************************/

std::string normal = "{ __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }";
std::string withSuper =
    "{ __THIS__: { __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }, __SUPER__: { "
    "__CLASS_NAME__: N2mi6MotherE, age: 10, name: \"test1\" } }";
std::string withMultipleSuper =
    "{ __THIS__: { __CLASS_NAME__: N2mi9Daughter2E, jobName: \"job\" }, "
    "__SUPER__: { __THIS__: { __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }, "
    "__SUPER__: { __CLASS_NAME__: N2mi6MotherE, age: 10, name: \"test2\" } } }";

std::string withSuper_thisValue =
    "{ __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }";
std::string withSuper_superValue =
    "{ __CLASS_NAME__: N2mi6MotherE, age: 10, name: \"test1\" }";

std::string withMultipleSuper_thisValue =
    "{ __CLASS_NAME__: N2mi9Daughter2E, jobName: \"job\" }";
std::string withMultipleSuper_superValue =
    "{ __THIS__: { __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }, __SUPER__: { "
    "__CLASS_NAME__: N2mi6MotherE, age: 10, name: \"test2\" } }";
std::string withMultipleSuper_superValue_thisValue =
    "{ __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }";
std::string withMultipleSuper_superValue_superValue =
    "{ __CLASS_NAME__: N2mi6MotherE, age: 10, name: \"test2\" }";

/******************************************************************************/
/*                           test for getThisValue                            */
/******************************************************************************/

TEST_CASE("getThisValue") {
    REQUIRE(getThisValue(normal) == normal);
    REQUIRE(getThisValue(withSuper) == withSuper_thisValue);
    REQUIRE(getThisValue(withMultipleSuper) == withMultipleSuper_thisValue);
}

/******************************************************************************/
/*                           test for getSuperValue                           */
/******************************************************************************/

TEST_CASE("getSuperValue") {
    REQUIRE(getSuperValue(normal) == "");
    REQUIRE(getSuperValue(withSuper) == withSuper_superValue);
    REQUIRE(getSuperValue(withMultipleSuper) == withMultipleSuper_superValue);
    REQUIRE(getThisValue(getSuperValue(withMultipleSuper)) ==
            withMultipleSuper_superValue_thisValue);
    REQUIRE(getSuperValue(getSuperValue(withMultipleSuper)) ==
            withMultipleSuper_superValue_superValue);
}
