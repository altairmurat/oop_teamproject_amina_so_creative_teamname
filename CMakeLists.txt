# Owner: Seunggyun (Person A)
cmake_minimum_required(VERSION 3.10)
project(ATMTermProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(atm ${SOURCES})

add_executable(test_bank
    src/Card.cpp src/Account.cpp src/Bank.cpp src/SystemLoader.cpp
    tests/bank_side_test.cpp)
