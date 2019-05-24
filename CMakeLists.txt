project(SimdTTM)
cmake_minimum_required(VERSION 3.5)

find_package(Vc 1.0)
if(Vc_FOUND)
    option(STTM_VC_ENABLED "Use Vc library" ON)
endif()

find_package(litesimd)
if(litesimd_FOUND)
    option(STTM_LITESIMD_ENABLED "Use litesimd library" OFF)
endif()

if(Vc_FOUND AND STTM_VC_ENABLED)
    message(STATUS "Using Vc libraries")
    set(SIMD_LINK_LIBRARY Vc::Vc)
    set(SIMD_DEFINE STTM_VC_ENABLED)
elseif(litesimd_FOUND AND STTM_LITESIMD_ENABLED)
    message(STATUS "Using litesimd libraries")
    set(SIMD_LINK_LIBRARY litesimd::litesimd)
    set(SIMD_DEFINE STTM_LITESIMD_ENABLED)
else()
    message(STATUS "Using SSE intrinsics")
    find_package(Boost 1.56.0 REQUIRED)
    set(SIMD_LINK_LIBRARY "")
    set(SIMD_DEFINE STTM_SSE_ENABLED)
endif()

option(STTM_USE_PREFETCH_OPTIMIZATION "Use memory prefetch optimization" ON)

set(COMPILE_FLAGS
        "-Wall"
        "-march=native"
        "-mtune=native"
        "-fno-strict-aliasing"
        "-D${SIMD_DEFINE}"
)

if(STTM_USE_PREFETCH_OPTIMIZATION)
    list(APPEND COMPILE_FLAGS "-DUSE_MEMORY_PREFETCH")
endif()
add_compile_options(${COMPILE_FLAGS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(bench)
add_subdirectory(test)
