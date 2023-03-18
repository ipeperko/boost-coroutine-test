cmake_minimum_required(VERSION 3.24)
project(cooroutine_test)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS context)

add_executable(cooroutine_test main.cpp)
target_link_libraries(cooroutine_test PRIVATE ${Boost_LIBRARIES})
