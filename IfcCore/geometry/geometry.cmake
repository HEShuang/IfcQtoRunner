set(
    GEOMETRY_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/geometry.cmake
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorBase.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorMesh.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorMesh.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorMeshFlow.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorMeshFlow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorOCC.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcElemProcessorOCC.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IfcGeometryParser.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcGeometryParser.cpp
)

source_group(geometry FILES ${GEOMETRY_SOURCES})
