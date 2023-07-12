cmake_minimum_required(VERSION 3.0)

project(test-ci)

add_executable(mytest mytest.cpp)
add_executable(test-mkdir test-mkdir.cpp)

enable_testing()

add_test(mytest mytest)
#add_test(test-mkdir test-mkdir)
