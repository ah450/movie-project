cmake_minimum_required(VERSION 2.8.8)
project(Movie-Organizer)
set(EXECUTABLE_NAME "movie-organizer")
find_package(Qt5 COMPONENTS Widgets Network Xml Sql REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
add_subdirectory(src)