cmake_minimum_required(VERSION 3.0.0)
set(TARGET_NAME simple)

project(${TARGET_NAME} VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)

FILE(GLOB src src/*.cpp src/*.cc )
FILE(GLOB inc inc/*.h)

add_executable(${TARGET_NAME} ${src} ${inc})
target_include_directories(${TARGET_NAME} PUBLIC inc/ ${CMAKE_SOURCE_DIR}/../boost_1_81_0/)


