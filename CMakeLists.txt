cmake_minimum_required(VERSION 3.25)
project(blockchain_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(blockchain_cpp
        hash_chain.cpp SHA/SHA256.cpp SHA/SHA256.h main.cpp)
