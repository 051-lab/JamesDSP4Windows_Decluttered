cmake_minimum_required(VERSION 3.16)
project(JamesDSP-GUI VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 path
set(CMAKE_PREFIX_PATH "D:/Qt/6.10.1/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

# JamesDSP library - COMPILE FROM SOURCE to enable debugging/fixes
# JamesDSP library - COMPILE FROM SOURCE to enable debugging/fixes
get_filename_component(JDSP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JDSP4Linux/libjamesdsp/subtree/Main/libjamesdsp/jni/jamesdsp" ABSOLUTE)

# Gather all C sources from the library
file(GLOB_RECURSE JDSP_SOURCES "${JDSP_PATH}/*.c")
list(FILTER JDSP_SOURCES EXCLUDE REGEX ".*eel2/cpthread.c")
message(STATUS "Found JamesDSP Sources: ${JDSP_SOURCES}")

# GUI sources - clean Windows-native implementation
set(GUI_SOURCES
    gui/main.cpp
    gui/MainWindow.cpp
    gui/MainWindow.h
    gui/widgets.h
    gui/curve_widgets.h
    gui/DspController.cpp
    gui/DspController.h
    gui/AudioService.cpp
    gui/AudioService.h
    gui/libs_impl.c
    src/DspConfig.cpp
    src/DspConfig.h
)

# Main executable (Include Lib Sources)
add_executable(JamesDSP-GUI WIN32 ${GUI_SOURCES} ${JDSP_SOURCES})

target_include_directories(JamesDSP-GUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compat
    ${JDSP_PATH}
    ${JDSP_PATH}/jdsp
    ${JDSP_PATH}/jdsp/Effects/eel2
    "${CMAKE_CURRENT_SOURCE_DIR}/../JDSP4Linux/libjamesdsp"
)

target_link_libraries(JamesDSP-GUI PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    ole32
    winmm
    mmdevapi
)

target_compile_definitions(JamesDSP-GUI PRIVATE WIN32 _WIN32)

# Copy Qt DLLs for development
add_custom_command(TARGET JamesDSP-GUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE:Qt6::Gui>
        $<TARGET_FILE_DIR:JamesDSP-GUI>
)

# Also copy Qt platform plugin
add_custom_command(TARGET JamesDSP-GUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:JamesDSP-GUI>/platforms
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll"
        $<TARGET_FILE_DIR:JamesDSP-GUI>/platforms/
)
