
cmake_minimum_required(VERSION 3.12)
project(LogAnalyzer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/LogAnalyzer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
