set(
    SERIALIZER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/serializer.cmake
    ${CMAKE_CURRENT_LIST_DIR}/IfcParseHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.h
    ${CMAKE_CURRENT_LIST_DIR}/IfcPreview.cpp
)

source_group(serializer FILES ${SERIALIZER_SOURCES})
