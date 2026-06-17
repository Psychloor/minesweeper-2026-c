cmake_minimum_required(VERSION 3.31)
project(minesweeper-c C)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

add_executable(minesweeper-c WIN32
        main.c
        Tile.h
        minefield.h
        minefield.c
        vector.h
        set.h
        settings.h
        settings.c)

target_compile_definitions(minesweeper-c PRIVATE NOMINMAX)

target_link_libraries(minesweeper-c PRIVATE SDL3::SDL3)

add_custom_command(TARGET minesweeper-c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:minesweeper-c>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/settings.ini"
        "$<TARGET_FILE_DIR:minesweeper-c>/settings.ini"
)