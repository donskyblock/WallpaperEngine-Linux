# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "src/app/CMakeFiles/wpe-linux_autogen.dir/AutogenUsed.txt"
  "src/app/CMakeFiles/wpe-linux_autogen.dir/ParseCache.txt"
  "src/app/wpe-linux_autogen"
  "src/cli/CMakeFiles/wpectl_autogen.dir/AutogenUsed.txt"
  "src/cli/CMakeFiles/wpectl_autogen.dir/ParseCache.txt"
  "src/cli/wpectl_autogen"
  "src/core/CMakeFiles/wpecore_autogen.dir/AutogenUsed.txt"
  "src/core/CMakeFiles/wpecore_autogen.dir/ParseCache.txt"
  "src/core/wpecore_autogen"
  "src/host/CMakeFiles/wpe-wallpaper-host_autogen.dir/AutogenUsed.txt"
  "src/host/CMakeFiles/wpe-wallpaper-host_autogen.dir/ParseCache.txt"
  "src/host/wpe-wallpaper-host_autogen"
  "tests/CMakeFiles/test_environment_autogen.dir/AutogenUsed.txt"
  "tests/CMakeFiles/test_environment_autogen.dir/ParseCache.txt"
  "tests/CMakeFiles/test_library_autogen.dir/AutogenUsed.txt"
  "tests/CMakeFiles/test_library_autogen.dir/ParseCache.txt"
  "tests/test_environment_autogen"
  "tests/test_library_autogen"
  )
endif()
