cmake_minimum_required (VERSION 2.6)
project (OpenVDBFun)

aux_source_directory(src/ FILES)

add_executable (run ${FILES})

add_definitions("-g")
add_definitions("-Wall")
add_definitions("-std=c++11")
#add_definitions("-O3")
add_definitions("-pthread")

target_link_libraries(run openvdb)
target_link_libraries(run noise)
target_link_libraries(run tbb)
target_link_libraries(run Half)


