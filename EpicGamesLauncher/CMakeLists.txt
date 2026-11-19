cmake_minimum_required(VERSION 3.6)

project(EpicGamesLauncher)

add_link_options(${LINK_OPTIONS})
add_executable(EpicGamesLauncher ${CMAKE_SOURCE_DIR}/main.c)

target_link_libraries(EpicGamesLauncher ${WIN_LIBRARIES})
