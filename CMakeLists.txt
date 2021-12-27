project(test-ci)

cmake_minimum_required(VERSION 3.0)

add_executable(mytest mytest.cpp)

enable_testing()

add_test(mytest mytest)
