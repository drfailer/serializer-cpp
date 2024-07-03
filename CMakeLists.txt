cmake_minimum_required(VERSION 3.16.3..3.28.1)

project(serializer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Wextra -g)

################################################################################
# variables                                                                    #
################################################################################

set(serializer_files
    ${CMAKE_SOURCE_DIR}/serializer/parser.hpp
    ${CMAKE_SOURCE_DIR}/serializer/serializer.hpp
    ${CMAKE_SOURCE_DIR}/serializer/attrcontainer.hpp
    ${CMAKE_SOURCE_DIR}/serializer/convertor/convertor.hpp
    ${CMAKE_SOURCE_DIR}/serializer/convertor/convert.hpp
    )

set(serializer_test_files
    ${CMAKE_SOURCE_DIR}/test/catch.cpp
    ${CMAKE_SOURCE_DIR}/test/test.cpp
)

set(parser_test_files
    ${CMAKE_SOURCE_DIR}/test/catch.cpp
    ${CMAKE_SOURCE_DIR}/test/test_parser.cpp
    )

include_directories(${CMAKE_SOURCE_DIR})

################################################################################
# executable                                                                   #
################################################################################

add_executable(serializer-tests ${serializer_test_files} ${serializer_files})
# add_executable(parser-tests ${parser_test_files} ${serializer_files})

################################################################################
# ctest                                                                        #
################################################################################

include(CTest)
add_test(NAME serializer COMMAND serializer-tests)
# add_test(NAME parser COMMAND parser-tests)
