cmake_minimum_required(VERSION 3.16)
project(AxiomJamesDSPConsole LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(JDSP_PATH "${REPO_ROOT}/JDSP4Linux/libjamesdsp/subtree/Main/libjamesdsp/jni/jamesdsp")
set(WIN_APP "${REPO_ROOT}/JamesDSP-Windows")

file(GLOB_RECURSE JDSP_SOURCES "${JDSP_PATH}/*.c")
list(FILTER JDSP_SOURCES EXCLUDE REGEX ".*eel2/cpthread.c")

add_executable(AxiomJamesDSPConsole
    "${WIN_APP}/src/main.cpp"
    "${WIN_APP}/src/DspConfig.cpp"
    ${JDSP_SOURCES}
)

target_include_directories(AxiomJamesDSPConsole PRIVATE
    "${WIN_APP}/src"
    "${WIN_APP}/src/compat"
    "${JDSP_PATH}"
    "${JDSP_PATH}/jdsp"
    "${JDSP_PATH}/jdsp/Effects/eel2"
    "${REPO_ROOT}/JDSP4Linux/libjamesdsp"
)

target_compile_definitions(AxiomJamesDSPConsole PRIVATE
    WIN32
    _WIN32
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(AxiomJamesDSPConsole PRIVATE /EHsc /bigobj)
endif()

target_link_libraries(AxiomJamesDSPConsole PRIVATE
    avrt
    ole32
    winmm
    mmdevapi
)
