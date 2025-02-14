cmake_minimum_required(VERSION 3.23)

project(
    weather_display
    VERSION 1.0
    DESCRIPTION "C++ Year 2024 ITMO labwork"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(lib)
add_subdirectory(bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
