project(VcAlgo)
cmake_minimum_required(VERSION 3.5)

find_package(Vc 1.0 REQUIRED)

if(MSVC)
    add_compile_options(
        "/W4"
        "$<$<CONFIG:RELEASE>:/O2>"
        "$<$<CONFIG:RELEASE>:/Oi>"
        "/MD$<$<CONFIG:Debug>:d>"
    )
else()
    add_compile_options(
        "-Wall"
        "-march=native"
        "-mtune=native"
        "-fno-strict-aliasing"
    )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(bench)
add_subdirectory(test)
