set(
    PARSE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/parse.cmake
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IfcSchemaStrategyBase.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcSchemaStrategyImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcStructureBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IfcStructureBuilder.h
)

source_group(parse FILES ${PARSE_SOURCES})
