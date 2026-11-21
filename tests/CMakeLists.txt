# SPDX-License-Identifier: GPL-3.0-or-later
qt_add_executable(test_environment test_environment.cpp)
target_link_libraries(test_environment PRIVATE wpecore Qt6::Test)
add_test(NAME environment COMMAND test_environment)

qt_add_executable(test_library test_library.cpp)
target_link_libraries(test_library PRIVATE wpecore Qt6::Test)
add_test(NAME library COMMAND test_library)

qt_add_executable(test_native test_native.cpp)
target_link_libraries(test_native PRIVATE wpecore Qt6::Test)
add_test(NAME native COMMAND test_native)

qt_add_executable(test_creator test_creator.cpp ../src/app/appcontroller.cpp ../src/app/models.cpp)
target_include_directories(test_creator PRIVATE ../src/app)
target_compile_definitions(test_creator PRIVATE SOURCE_ROOT="${CMAKE_SOURCE_DIR}")
qt_add_resources(test_creator native_canvas PREFIX "/shared" BASE ${CMAKE_SOURCE_DIR}/src/shared FILES ${CMAKE_SOURCE_DIR}/src/shared/NativeCanvas.qml)
target_link_libraries(test_creator PRIVATE wpecore Qt6::Test Qt6::Widgets Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia)
add_test(NAME creator COMMAND test_creator)

qt_add_executable(test_widgets test_widgets.cpp)
target_link_libraries(test_widgets PRIVATE wpecore Qt6::Test)
add_test(NAME widgets COMMAND test_widgets)

qt_add_executable(test_widget_dbus test_widget_dbus.cpp)
target_link_libraries(test_widget_dbus PRIVATE wpecore Qt6::Test)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
add_test(NAME widget_dbus COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:test_widget_dbus>)
