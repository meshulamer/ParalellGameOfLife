cmake_minimum_required(VERSION 3.15)
project(p1)

set(CMAKE_CXX_STANDARD 11)

add_executable(p1 main.cpp Semaphore.cpp PCQueue.hpp threadPool.h)