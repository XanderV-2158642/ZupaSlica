cmake_minimum_required(VERSION 3.5.0)
project(ZupaSlica VERSION 0.1.0 LANGUAGES C CXX)

add_executable(ZupaSlica src/main.cpp src/glad.c)

target_link_libraries(ZupaSlica glfw)
target_include_directories(ZupaSlica PRIVATE ${ZupaSlica_SOURCE_DIR}/include)
