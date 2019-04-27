
# Boost is requirement for the benchmark
find_package(Boost 1.56.0 COMPONENTS timer chrono system)

if(Boost_FOUND)
    add_subdirectory(binary_search)
    add_subdirectory(btree)
endif()
