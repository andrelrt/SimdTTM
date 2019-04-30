# Enable ExternalProject CMake module
include(ExternalProject)

list(APPEND CPPBTREE_CMAKE_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
list(APPEND CPPBTREE_CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
list(APPEND CPPBTREE_CMAKE_ARGS "-Dbuild_bench=OFF")
if(MSVC)
    list(APPEND CPPBTREE_CMAKE_ARGS "-DBUILD_SHARED_LIBS=ON")
endif()

# Download and install GoogleTest
ExternalProject_Add(
    cpp-btree
    URL https://github.com/andrelrt/cpp-btree/archive/v1.0.1.tar.gz
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/cpp-btree
    # Disable install step
    INSTALL_COMMAND ""
    CMAKE_ARGS ${CPPBTREE_CMAKE_ARGS}
)
ExternalProject_Get_Property(cpp-btree source_dir binary_dir)

project(btree_set)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME}
	${SRC_LIST}
)

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
    "${source_dir}/include"

	SYSTEM PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Boost_LIBRARIES}
    ${SIMD_LINK_LIBRARY}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
