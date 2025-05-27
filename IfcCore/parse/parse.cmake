set(
    PARSE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/parse.cmake
    ${CMAKE_CURRENT_LIST_DIR}/IfcParseHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.cpp
)

source_group(parse FILES ${PARSE_SOURCES})
