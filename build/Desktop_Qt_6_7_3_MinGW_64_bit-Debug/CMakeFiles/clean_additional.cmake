# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\LuckDev1_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\LuckDev1_autogen.dir\\ParseCache.txt"
  "LuckDev1_autogen"
  )
endif()
