cmake_minimum_required(VERSION 3.5)

project(HikProtocolLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(boost_base)

find_package(OpenSSL REQUIRED)

add_library(${PROJECT_NAME} STATIC "")

file(GLOB_RECURSE PRJ_SRCS src/*.cpp src/*.h src/*.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${PRJ_SRCS})
target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::Crypto boost_base)
target_include_directories(${PROJECT_NAME} PUBLIC src/)

#install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
