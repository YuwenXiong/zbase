cmake_minimum_required(VERSION 3.3)
project(zbase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp PF.h zbase.h PF_Manager.cpp PF_BufferManager.cpp PF_BufferManager.h PF_HashTable.cpp PF_HashTable.h PF_PageHandle.cpp PF_FileHandle.cpp QL.h SM.h RM.h IX.h IX.cpp RM.cpp parser.h common.h zbase.cpp.cpp zbase.cpp.h zbase.cpp.cpp zbase.cpp parser.cpp)
add_executable(zbase ${SOURCE_FILES})