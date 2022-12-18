cmake_minimum_required(VERSION 3.5)

project(webserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(webserver main.cpp webserver.h webserver.cpp)

install(TARGETS webserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
