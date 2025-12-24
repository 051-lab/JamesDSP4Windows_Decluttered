# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\JamesDSP-GUI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\JamesDSP-GUI_autogen.dir\\ParseCache.txt"
  "JamesDSP-GUI_autogen"
  )
endif()
