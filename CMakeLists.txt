cmake_minimum_required(VERSION 3.15)
project(graphs CXX)
add_executable(graphs main.cpp)

set_property(TARGET graphs PROPERTY CXX_STANDARD 20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Osmium REQUIRED COMPONENTS pbf)
if (NOT OSMIUM_FOUND)
    message(WARNING "Libosmium not found\n")
endif ()

find_package(ZLIB REQUIRED)
if (NOT ZLIB_FOUND)
    message(WARNING "Zlib not found\n")
endif ()

find_package(Threads REQUIRED)
if (NOT THREADS_FOUND)
    message(WARNING "Pthreads not found\n")
endif ()

configure_file(data/NNMap.pbf ${CMAKE_CURRENT_BINARY_DIR}/NNMap.pbf COPYONLY)

target_compile_options(graphs PRIVATE -Werror -Wall -Wextra)

target_link_libraries(graphs PUBLIC ${OSMIUM_IO_LIBRARIES}
                                    ${ZLIB_LIBRARIES}
                                    ${CMAKE_THREAD_LIBS_INIT})
