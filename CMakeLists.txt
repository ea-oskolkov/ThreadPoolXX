cmake_minimum_required(VERSION 3.20)

project(ThreadPoolXXLib
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)

set(PROJECT_VERSION 0.0.1)

set(DIR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DIR_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCE_FILES ${DIR_SOURCE}/*.cpp)
include_directories(${DIR_INCLUDE})

if (${BUILD_SHARED_LIBS})
    add_library(ThreadPoolXXLib SHARED ${SOURCE_FILES})
else()
    add_library(ThreadPoolXXLib STATIC ${SOURCE_FILES})
endif()
