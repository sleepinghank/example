cmake_minimum_required(VERSION 3.28)

project(cplus)

#set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(cplus
        include/inheritance.h
        src/inheritance.cpp
        include/functions.h
        src/functions.cpp
        main.cpp
        )

