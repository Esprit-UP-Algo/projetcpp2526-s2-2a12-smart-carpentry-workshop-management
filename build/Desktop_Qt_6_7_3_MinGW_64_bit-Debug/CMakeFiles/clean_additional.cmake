# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\WoodFlow_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\WoodFlow_autogen.dir\\ParseCache.txt"
  "WoodFlow_autogen"
  )
endif()
