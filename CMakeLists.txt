cmake_minimum_required(VERSION 3.0)

project(demo)

set(SOURCE_EXE main.c)
set(SOURCE_TYPES types.c)

add_library(types STATIC ${SOURCE_TYPES})
add_executable(main ${SOURCE_EXE})

set_property(TARGET types PROPERTY C_STANDARD 99)
set_property(TARGET main PROPERTY C_STANDARD 99)

target_link_libraries(main types)

find_package(SDL REQUIRED)