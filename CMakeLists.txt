cmake_minimum_required(VERSION 3.12)
project(graphs CXX)

set(SOURCE ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE ${PROJECT_SOURCE_DIR}/include)
set(DATA ${PROJECT_SOURCE_DIR}/data)

set(GRAPHS_SOURCES ${SOURCE}/assessment.cpp
                   ${SOURCE}/planning.cpp
                   ${SOURCE}/graph.cpp
                   ${SOURCE}/main.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_executable(graphs ${GRAPHS_SOURCES})

target_include_directories(graphs PRIVATE ${SOURCE})
target_include_directories(graphs PRIVATE ${INCLUDE})

set_property(TARGET graphs PROPERTY CXX_STANDARD 20)

# Required dynamic-link libraries.
find_package(Osmium REQUIRED COMPONENTS pbf)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# Move necessary data files to build directory.
configure_file(data/NNMap.pbf ${CMAKE_CURRENT_BINARY_DIR}/NNMap.pbf COPYONLY)

target_compile_options(graphs PRIVATE -Werror -Wall -Wextra -g)

target_link_libraries(graphs PUBLIC ${OSMIUM_IO_LIBRARIES}
                                    ${ZLIB_LIBRARIES}
                                    ${CMAKE_THREAD_LIBS_INIT})
