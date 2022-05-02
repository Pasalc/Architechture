cmake_minimum_required(VERSION 3.2)

project(lab02 C CXX)

SET (SERVER "server")
SET (U_TESTS "unittests")

find_package(Threads)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON)
find_package(GTest REQUIRED)
find_package(ZLIB)

if (NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
include_directories(${Poco_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${GTEST_INCLUDE_DIRS})
link_directories("/usr/local/lib")

add_executable(${SERVER} server.cpp)
add_executable(${U_TESTS} unittests.cpp)

target_include_directories(${SERVER} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${SERVER} PRIVATE)
target_link_libraries(${SERVER} PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${Poco_LIBRARIES} "PocoData" "PocoDataMySQL" "mysqlclient" ZLIB::ZLIB)

set_target_properties(${SERVER} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${SERVER} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(${U_TESTS} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${U_TESTS} PRIVATE)
target_link_libraries(${U_TESTS} PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${Poco_LIBRARIES} ${GTEST_LIBRARIES} "PocoData" "PocoDataMySQL" "mysqlclient" ZLIB::ZLIB)
set_target_properties(${U_TESTS} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${U_TESTS} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
