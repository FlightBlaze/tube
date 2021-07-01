cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

project("tube")

set(
    TUBE_SOURCES
    "include/Tube.h"
    "include/Path.h"
    "source/Tube.cpp"
    "source/Path.cpp")

set(
    GLM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../glm/" )

if (NOT TARGET glm)
    add_subdirectory(${GLM_PATH} "${CMAKE_CURRENT_BINARY_DIR}/glm")
endif()

add_library(
             tube
             ${TUBE_SOURCES} )

target_include_directories( tube PRIVATE
        "include/"
        ${GLM_PATH} )

target_link_libraries(
                       tube
                       glm)
