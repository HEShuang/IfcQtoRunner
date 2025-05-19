# FindIfcOpenShell
# ----------
#
# Find Ifc Open Shell library, this modules defines:
#
# IFCOPENSHELL_INCLUDE_DIRS, where to find Ifc Open Shell include files
# IFCOPENSHELL_LIBRARIES, where to find library
# IFCOPENSHELL_FOUND, if it is found

#Look for Ifc Geom.
find_path(GEOM_INCLUDE_DIR NAMES ifcgeom/IfcGeomElement.h)
find_library(GEOM_LIBRARY NAMES libIfcGeom IfcGeom)

#Look for Ifc Parse.
find_path(PARSE_INCLUDE_DIR NAMES ifcparse/IfcParse.h)
find_library(PARSE_LIBRARY NAMES libIfcParse IfcParse)

#Set IFCOPENSHELL_FOUND.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	IfcOpenShell
	FOUND_VAR IFCOPENSHELL_FOUND
	REQUIRED_VARS GEOM_INCLUDE_DIR GEOM_LIBRARY PARSE_INCLUDE_DIR PARSE_LIBRARY
)

#Set IFCOPENSHELL_INCLUDE_DIRS and IFCOPENSHELL_LIBRARIES.
if(IFCOPENSHELL_FOUND)
	set(IFCOPENSHELL_INCLUDE_DIRS ${GEOM_INCLUDE_DIR} ${PARSE_INCLUDE_DIR})
	set(IFCOPENSHELL_LIBRARIES ${GEOM_LIBRARY} ${PARSE_LIBRARY})
endif ()

#Mark working variables as advanced so they won't be visible to end user.
mark_as_advanced(GEOM_INCLUDE_DIR GEOM_LIBRARY PARSE_INCLUDE_DIR PARSE_LIBRARY)