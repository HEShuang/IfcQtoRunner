# Usage:
#   set(OCC_FIND_COMPONENTS TKernel TKMath ...)
#   find_package(OCC REQUIRED)

include(FindPackageHandleStandardArgs)

find_path(OCC_INCLUDE_DIR
  NAMES GeomAPI.hxx
  PATHS
    ENV OPENCASCADE_INCLUDE_DIR
    ${OCC_ROOT}/include
    ${OCC_ROOT}/include/opencascade
    /opt/homebrew/include/opencascade
    /usr/local/include/opencascade
    /usr/include/opencascade
)

set(OCC_INCLUDE_DIRS ${OCC_INCLUDE_DIR})

# Default components if none are specified
if(NOT OCC_FIND_COMPONENTS)
  set(OCC_FIND_COMPONENTS TKernel TKMath TKGeomBase TKGeomAlgo TKG3d TKG2d TKBRep TKTopAlgo TKShHealing TKMesh TKPrim TKBool TKBO TKFillet TKOffset)
endif()

set(OCC_LIBRARIES "")
foreach(component IN LISTS OCC_FIND_COMPONENTS)
  find_library(OCC_${component}_LIBRARY
    NAMES ${component} lib${component}
    PATHS
      ENV OCC_LIB_DIR
      ${OCC_ROOT}/lib
      ${OCC_ROOT}/lib64
      /opt/homebrew/lib
      /usr/local/lib
      /usr/lib
  )

  if(OCC_${component}_LIBRARY)
    list(APPEND OCC_LIBRARIES ${OCC_${component}_LIBRARY})
    set(OCC_${component}_FOUND TRUE)
    mark_as_advanced(OCC_${component}_LIBRARY)
  else()
    set(OCC_${component}_FOUND FALSE)
  endif()
endforeach()

# Report
find_package_handle_standard_args(OCC
  REQUIRED_VARS OCC_INCLUDE_DIR
  HANDLE_COMPONENTS
)

mark_as_advanced(OCC_INCLUDE_DIR)
