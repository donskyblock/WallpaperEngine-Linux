# SPDX-License-Identifier: GPL-3.0-or-later
add_library(wpecore STATIC
    types.cpp environment.cpp settings.cpp tools.cpp library.cpp workshop.cpp steamcmd.cpp
    nativeapply.cpp wallpapermanager.cpp)
target_include_directories(wpecore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wpecore PUBLIC Qt6::Core Qt6::Gui Qt6::Network Qt6::DBus)
set_target_properties(wpecore PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_options(wpecore PRIVATE -Wall -Wextra)
