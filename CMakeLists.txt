cmake_minimum_required(VERSION 3.7)
project(VM)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp MicroJavaLib.cpp MicroJavaLib.h FrameStack.h)
add_executable(VM ${SOURCE_FILES})