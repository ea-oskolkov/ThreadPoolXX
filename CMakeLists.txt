cmake_minimum_required(VERSION 3.20)

project(ThreadPoolXX
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_VERSION 0.0.1)

set(DIR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DIR_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(DIR_EXAMPLES "${CMAKE_CURRENT_SOURCE_DIR}/examples")

file(GLOB_RECURSE SOURCE_FILES ${DIR_SOURCE}/*.cpp)
include_directories(${DIR_INCLUDE})

if (${BUILD_SHARED_LIBS})
    add_library(ThreadPoolXX SHARED ${SOURCE_FILES})
else()
    add_library(ThreadPoolXX STATIC ${SOURCE_FILES})
endif()


# =========================
# Example support
# =========================
find_package(Threads REQUIRED)
target_link_libraries(ThreadPoolXX PRIVATE Threads::Threads)

option(BUILD_EXAMPLES "Build example(s)" OFF)

if (BUILD_EXAMPLES)
    add_executable(example_main ${DIR_EXAMPLES}/main.cpp)
    target_include_directories(example_main PRIVATE ${DIR_INCLUDE})
    target_link_libraries(example_main PRIVATE ThreadPoolXX)
endif()

# =========================
# Installation support
# =========================
include(GNUInstallDirs) # provides CMAKE_INSTALL_LIBDIR, etc.
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_CMAKE_DIR ${INSTALL_LIB_DIR}/cmake/ThreadPoolXX)

# install library and export targets
install(TARGETS ThreadPoolXX
    EXPORT ThreadPoolXXTargets
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR}
)

# install headers (install include/ThreadPoolXX directory)
install(DIRECTORY ${DIR_INCLUDE}/ DESTINATION ${INSTALL_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.hh"
)
