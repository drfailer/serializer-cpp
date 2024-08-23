cmake_minimum_required(VERSION 3.16.3..3.28.1)

project(serializer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Wextra -g -Winline)
# add_compile_options(-Wall -Wextra -O3)

################################################################################
# variables                                                                    #
################################################################################

set(serializer_files
  serializer/convertor/convertor.hpp
  serializer/convertor/convert.hpp
  serializer/exceptions/unsupported_type.hpp
  serializer/exceptions/unknown_specialized_type.hpp
  serializer/tools/tools.hpp
  serializer/tools/id_table.hpp
  serializer/tools/vec.hpp
  serializer/tools/context.hpp
  serializer/tools/dynamic_array.hpp
  serializer/meta/concepts.hpp
  serializer/meta/type_check.hpp
  serializer/meta/type_transform.hpp
  serializer/serialize.hpp
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
