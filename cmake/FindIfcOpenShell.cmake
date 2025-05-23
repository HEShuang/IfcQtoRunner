# FindIfcOpenShell
#
# IFCOPENSHELL_FOUND
# IFCOPENSHELL_INCLUDE_DIRS
# IFCOPENSHELL_LIBRARIES

include(FindPackageMessage)

find_path(PARSE_INCLUDE_DIR NAMES ifcparse/IfcParse.h)
find_library(PARSE_LIBRARY NAMES libIfcParse IfcParse)

get_filename_component(LIB_DIR ${PARSE_LIBRARY} DIRECTORY)
file(GLOB ALL_LIBS
	"${LIB_DIR}/lib*.a"
	"${LIB_DIR}/lib*.so"
	"${LIB_DIR}/lib*.dylib"
	"${LIB_DIR}/lib*.lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	IfcOpenShell
	FOUND_VAR IFCOPENSHELL_FOUND
	REQUIRED_VARS PARSE_INCLUDE_DIR PARSE_LIBRARY
)

if(IFCOPENSHELL_FOUND)
	set(IFCOPENSHELL_INCLUDE_DIRS ${PARSE_INCLUDE_DIR})
	set(IFCOPENSHELL_LIBRARIES ${ALL_LIBS})
endif ()

#Hide local variables
mark_as_advanced(PARSE_INCLUDE_DIR PARSE_LIBRARY ALL_LIBS)
