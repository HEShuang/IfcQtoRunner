cmake_minimum_required(VERSION 3.16)

project(IfcCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IFCOPENSHELL_PREFIX "/Users/she/MyLibs/IfcOpenShell/Debug/usr/local")
else()
  set(IFCOPENSHELL_PREFIX "/Users/she/MyLibs/IfcOpenShell/Release/usr/local")
endif()

list(APPEND CMAKE_PREFIX_PATH "${IFCOPENSHELL_PREFIX}")

set(Eigen_INCLUDE_DIR "/opt/homebrew/include/eigen3")

find_package(IfcOpenShell REQUIRED)
find_package(Boost)
find_package(OCC REQUIRED)

message(STATUS "IfcOpenShell inlcude dir: ${IFCOPENSHELL_INCLUDE_DIRS}")
message(STATUS "IfcOpenShell libs: ${IFCOPENSHELL_LIBRARIES}")
message(STATUS "OCC include: ${OCC_INCLUDE_DIRS}")
message(STATUS "OCC libs: ${OCC_LIBRARIES}")

include(model/model.cmake)
include(serializer/serializer.cmake)

add_library(IfcCore STATIC
  ${MODEL_SOURCES}
  ${PARSE_SOURCES}
)

target_include_directories(IfcCore
  PUBLIC
    ${Eigen_INCLUDE_DIR}
    ${IFCOPENSHELL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${OCC_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/model>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/parse>
)
target_link_libraries(IfcCore
  PRIVATE
    ${IFCOPENSHELL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OCC_LIBRARIES}
)

target_compile_definitions(IfcCore PRIVATE IFCENGINE_LIBRARY_BUILD)
