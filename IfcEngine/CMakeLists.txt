cmake_minimum_required(VERSION 3.16)

project(IfcEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IFCOPENSHELL_PREFIX "/Users/she/MyLibs/IfcOpenShell/Debug/usr/local")
else()
  set(IFCOPENSHELL_PREFIX "/Users/she/MyLibs/IfcOpenShell/Release/usr/local")
endif()

list(APPEND CMAKE_PREFIX_PATH "${IFCOPENSHELL_PREFIX}")

message(STATUS "Configuring for ${CMAKE_BUILD_TYPE}, "
               "adding ${IFCOPENSHELL_PREFIX} to CMAKE_PREFIX_PATH")

find_package(IfcOpenShell REQUIRED)
find_package(Boost)
find_package(OCC)

include(model/model.cmake)
include(serializer/serializer.cmake)

add_library(IfcEngine STATIC ${MODEL_SOURCES} ${SERIALIZER_SOURCES})

target_include_directories(IfcEngine
  PUBLIC
    ${IFCOPENSHELL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${OCC_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/model>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/serializer>
)
target_link_libraries(IfcEngine
  PRIVATE
    ${IFCOPENSHELL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OCC_LIBRARIES}
)

target_compile_definitions(IfcEngine PRIVATE IFCENGINE_LIBRARY_BUILD)
