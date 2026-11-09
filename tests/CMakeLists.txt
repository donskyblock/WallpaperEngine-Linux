# SPDX-License-Identifier: GPL-3.0-or-later
qt_add_executable(test_environment test_environment.cpp)
target_link_libraries(test_environment PRIVATE wpecore Qt6::Test)
add_test(NAME environment COMMAND test_environment)

qt_add_executable(test_library test_library.cpp)
target_link_libraries(test_library PRIVATE wpecore Qt6::Test)
add_test(NAME library COMMAND test_library)
