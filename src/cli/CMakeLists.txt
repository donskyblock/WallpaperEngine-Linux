# SPDX-License-Identifier: GPL-3.0-or-later
qt_add_executable(wpectl main.cpp)
target_link_libraries(wpectl PRIVATE Qt6::Core Qt6::DBus)
install(TARGETS wpectl DESTINATION ${CMAKE_INSTALL_BINDIR})
