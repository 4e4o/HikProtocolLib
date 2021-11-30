cmake_minimum_required(VERSION 3.5)

project(HikProtocolLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

add_library(${PROJECT_NAME} STATIC "")

file(GLOB_RECURSE PRJ_SRCS src/*.cpp src/*.h src/*.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${PRJ_SRCS})

#target_link_directories(${PROJECT_NAME} PRIVATE /mnt/disk2/workspace/coding/openssl/)
#target_link_libraries(${PROJECT_NAME} crypto Boost::system Boost::thread Boost::filesystem rt)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::Crypto Boost::thread rt)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PUBLIC src/)

#install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
