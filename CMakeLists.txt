cmake_minimum_required(VERSION 3.25)
project(blockchain_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(blockchain_cpp
        main.cpp SHA256.cpp SHA256.h Blockchain.cpp Blockchain.h)
