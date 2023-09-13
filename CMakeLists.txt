cmake_minimum_required(VERSION 3.25)

project(ThreadPoolXX
        VERSION 1.0.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_VERSION 0.0.1)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)

add_library(ThreadPoolXX ${SOURCE_FILES})
