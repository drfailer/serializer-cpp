#include "catch.hpp"
#include "serializer/tools/parser.hpp"
#include <map>
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
    REQUIRE(serializer::parser::getThisValue(normal) == normal);
    REQUIRE(serializer::parser::getThisValue(withSuper) == withSuper_thisValue);
    REQUIRE(serializer::parser::getThisValue(withMultipleSuper) ==
            withMultipleSuper_thisValue);
}

/******************************************************************************/
/*                           test for getSuperValue                           */
/******************************************************************************/

TEST_CASE("getSuperValue") {
    REQUIRE(serializer::parser::getSuperValue(normal) == "");
    REQUIRE(serializer::parser::getSuperValue(withSuper) ==
            withSuper_superValue);
    REQUIRE(serializer::parser::getSuperValue(withMultipleSuper) ==
            withMultipleSuper_superValue);
    REQUIRE(serializer::parser::getThisValue(serializer::parser::getSuperValue(
                withMultipleSuper)) == withMultipleSuper_superValue_thisValue);
    REQUIRE(serializer::parser::getSuperValue(serializer::parser::getSuperValue(
                withMultipleSuper)) == withMultipleSuper_superValue_superValue);
}

/******************************************************************************/
/*                            test parse one level                            */
/******************************************************************************/

TEST_CASE("parseOneLvl") {
    std::map<std::string, std::string> result;

    /* normal */
    result = serializer::parser::parseOneLvl(normal);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("__CLASS_NAME__") != result.end());
    REQUIRE(result.at("__CLASS_NAME__") == "N2mi9Daughter1E");
    REQUIRE(result.find("money") != result.end());
    REQUIRE(result.at("money") == "2.2");
    result.clear();

    /* with more values */
    result = serializer::parser::parseOneLvl(
        "{ number: 1, string: \"hello\", pair: { 1, 2 }, double: 3.3, array: [ "
        "1, 2, 3 ], objArray: [ { obj }, { obj } ] }");
    REQUIRE(result.size() == 6);
    REQUIRE(result.find("number") != result.end());
    REQUIRE(result.at("number") == "1");
    REQUIRE(result.find("string") != result.end());
    REQUIRE(result.at("string") == "\"hello\"");
    REQUIRE(result.find("pair") != result.end());
    REQUIRE(result.at("pair") == "{ 1, 2 }");
    REQUIRE(result.find("double") != result.end());
    REQUIRE(result.at("double") == "3.3");
    REQUIRE(result.find("array") != result.end());
    REQUIRE(result.at("array") == "[ 1, 2, 3 ]");
    REQUIRE(result.find("objArray") != result.end());
    REQUIRE(result.at("objArray") == "[ { obj }, { obj } ]");
    result.clear();

    /* with nested elements (should not happend normally but it is still
     * important) */
    result = serializer::parser::parseOneLvl(withMultipleSuper);
    REQUIRE(result.size() == 2);
    REQUIRE(result.find("__THIS__") != result.end());
    REQUIRE(result.at("__THIS__") ==
            "{ __CLASS_NAME__: N2mi9Daughter2E, jobName: \"job\" }");
    REQUIRE(result.find("__SUPER__") != result.end());
    REQUIRE(result.at("__SUPER__") ==
            "{ __THIS__: { __CLASS_NAME__: N2mi9Daughter1E, money: 2.2 }, "
            "__SUPER__: { __CLASS_NAME__: N2mi6MotherE, age: 10, name: "
            "\"test2\" } }");
    result.clear();
}

/******************************************************************************/
/*                               string escape                                */
/******************************************************************************/

TEST_CASE("escape string") {
    std::string str1 = "hello \" world";
    std::string str2 = "hello \"world\"";
    std::string str3 = "hello \"world\"\"";

    REQUIRE("hello \\\" world" ==
            serializer::parser::escapeStr("hello \" world"));
    REQUIRE("hello \\\"world\\\"" ==
            serializer::parser::escapeStr("hello \"world\""));
    REQUIRE("hello \\\"world\\\"\\\"" ==
            serializer::parser::escapeStr("hello \"world\"\""));

    REQUIRE(str1 == serializer::parser::unescapeStr("hello \\\" world"));
    REQUIRE(str2 == serializer::parser::unescapeStr("hello \\\"world\\\""));
    REQUIRE(str3 == serializer::parser::unescapeStr("hello \\\"world\\\"\\\""));

    REQUIRE(str1 == serializer::parser::unescapeStr(
                        serializer::parser::escapeStr(str1)));
    REQUIRE(str2 == serializer::parser::unescapeStr(
                        serializer::parser::escapeStr(str2)));
    REQUIRE(str3 == serializer::parser::unescapeStr(
                        serializer::parser::escapeStr(str3)));
}

/******************************************************************************/
/*                                parse tuple                                 */
/******************************************************************************/

TEST_CASE("parse tuple") {
    std::string tupleValue = "{ 14, \"string\", { objects }, 12 }";
    std::pair<std::string, std::string> content =
        serializer::parser::parseTuple(tupleValue);

    REQUIRE(content.first == "14");
    REQUIRE(content.second == ", \"string\", { objects }, 12 }");
    content = serializer::parser::parseTuple(content.second);

    REQUIRE(content.first == "\"string\"");
    REQUIRE(content.second == ", { objects }, 12 }");
    content = serializer::parser::parseTuple(content.second);

    REQUIRE(content.first == "{ objects }");
    REQUIRE(content.second == ", 12 }");
    content = serializer::parser::parseTuple(content.second);

    REQUIRE(content.first == "12");
}
