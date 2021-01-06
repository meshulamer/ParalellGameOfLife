cmake_minimum_required(VERSION 3.15)
project(Part2)

set(CMAKE_CXX_STANDARD 11)

add_executable(Part2 main.cpp ../Part1/Semaphore.cpp ../Part1/PCQueue.hpp Game.cpp utils.cpp ../Part2/Thread.hpp)