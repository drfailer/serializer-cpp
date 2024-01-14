cmake_minimum_required(VERSION 3.16.3..3.28.1)

project(serializer)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Wextra -g)

################################################################################
# variables                                                                    #
################################################################################

set(serializer_files
    ${CMAKE_SOURCE_DIR}/serializer/parser.hpp
    ${CMAKE_SOURCE_DIR}/serializer/convertor.hpp
    ${CMAKE_SOURCE_DIR}/serializer/serializer.hpp
    ${CMAKE_SOURCE_DIR}/serializer/attrcontainer.hpp
    )

set(test_files
    ${CMAKE_SOURCE_DIR}/test/catch.cpp
    ${CMAKE_SOURCE_DIR}/test/test.cpp
    ${CMAKE_SOURCE_DIR}/test/test_parser.cpp
    )

include_directories(${CMAKE_SOURCE_DIR})

################################################################################
# executable                                                                   #
################################################################################

add_executable(serializer-tests ${test_files} ${serializer_files})
