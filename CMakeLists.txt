cmake_minimum_required(VERSION 3.7)
project(mini-jvm)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp FrameStack.h)
add_executable(minijvm ${SOURCE_FILES})