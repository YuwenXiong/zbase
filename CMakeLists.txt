cmake_minimum_required(VERSION 3.3)
project(zbase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp PF.h zbase.h PF_Manager.cpp PF_Manager.h PF_BufferManager.cpp PF_BufferManager.h)
add_executable(zbase ${SOURCE_FILES})