cmake_minimum_required(VERSION 3.25)
project(blockchain_cpp)

set(CMAKE_CXX_STANDARD 20)

include_directories(SHA)

add_executable(blockchain_cpp
        SHA/SHA256.cpp
        SHA/SHA256.h
        main.cpp)
