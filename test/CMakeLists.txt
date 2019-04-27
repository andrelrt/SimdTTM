
find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

list(APPEND GTEST_CMAKE_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
list(APPEND GTEST_CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
if(MSVC)
    list(APPEND GTEST_CMAKE_ARGS "-DBUILD_SHARED_LIBS=ON")
endif()

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/release-1.8.1.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
    CMAKE_ARGS ${GTEST_CMAKE_ARGS}
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_library(libgtest_main IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
add_dependencies(libgtest_main gtest)

# Set libgtest properties
if(MSVC)
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/Release/gtest.lib"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)
else()
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)
set_target_properties(libgtest_main PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest_main.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)
endif()

# Create a libgmock target to be used as a dependency by test programs
#add_library(libgmock IMPORTED STATIC GLOBAL)
#add_dependencies(libgmock gtest)
#
## Set libgmock properties
#set_target_properties(libgmock PROPERTIES
#    "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
#    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#)

cmake_policy(SET CMP0054 NEW)
if(CMAKE_VERSION VERSION_LESS "3.10")
    cmake_policy(SET CMP0057 NEW)
endif()

if(CMAKE_VERSION VERSION_LESS "3.9")
    find_package(GTest)
else()
    include(GoogleTest)
endif()

# Test project
project(SimdTTMTest)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME}
	${SRC_LIST}
)

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
    "${source_dir}/googletest/include"

	PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    libgtest
    libgtest_main
    ${SIMD_LINK_LIBRARY}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

if(CMAKE_VERSION VERSION_LESS "3.9")
    GTEST_ADD_TESTS(${PROJECT_NAME} "" AUTO)
elseif(CMAKE_VERSION VERSION_LESS "3.10")
    gtest_add_tests(TARGET ${PROJECT_NAME})
else()
    gtest_discover_tests(${PROJECT_NAME})
endif()
