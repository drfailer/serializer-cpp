cmake_minimum_required(VERSION 3.16.3..3.28.1)

project(serializer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# add_compile_options(-Wall -Wextra -g)
add_compile_options(-Wall -Wextra -O3)

################################################################################
# variables                                                                    #
################################################################################

set(serializer_files
        serializer/serializer.hpp
        serializer/member_list.hpp
        serializer/convertor/convertor.hpp
        serializer/convertor/convert.hpp
        serializer/tools/tools.hpp
        serializer/tools/exceptions.hpp
        serializer/tools/metafunctions.hpp
        serializer/tools/concepts.hpp
)

set(serializer_test_files
        test/catch.cpp
        test/test.cpp
)

include_directories(${CMAKE_SOURCE_DIR})

################################################################################
# executable                                                                   #
################################################################################

add_executable(serializer-tests ${serializer_test_files} ${serializer_files})

################################################################################
# ctest                                                                        #
################################################################################

include(CTest)
add_test(NAME serializer COMMAND serializer-tests)
