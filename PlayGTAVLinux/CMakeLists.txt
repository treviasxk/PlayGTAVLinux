cmake_minimum_required(VERSION 3.16)

project(PlayGTAVLinux)

add_link_options(${LINK_OPTIONS})
add_executable(PlayGTAVLinux ${CMAKE_SOURCE_DIR}/main.c)

target_link_libraries(PlayGTAVLinux ${WIN_LIBRARIES})
