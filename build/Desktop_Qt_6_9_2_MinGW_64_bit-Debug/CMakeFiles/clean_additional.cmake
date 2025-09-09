# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\WeatherAnalyzer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\WeatherAnalyzer_autogen.dir\\ParseCache.txt"
  "WeatherAnalyzer_autogen"
  )
endif()
