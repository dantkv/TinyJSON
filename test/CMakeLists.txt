cmake_minimum_required(VERSION 3.13)

project(
    tinyjson_test
    LANGUAGES CXX
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

enable_testing()

find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
set(memcheck_command ${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})


add_library(tiny_jsonlib ${PROJECT_SOURCE_DIR}/../src/tiny_json.cpp)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/../src)

set(TEST_LIST
    constant_test
    strcmp_s_test
    number_test
    string_test
    func_test
    array_test
    object_test
)

foreach(src ${TEST_LIST})
	add_executable(${src} ${src}.cpp)
    target_link_libraries(${src} tiny_jsonlib gtest_main gtest pthread)
	add_test(${src} ${src})
	add_dependencies(check ${src})
endforeach()

foreach(src ${TEST_LIST})
	add_test(${src}-memory-check ${memcheck_command} ./${src})
endforeach()
