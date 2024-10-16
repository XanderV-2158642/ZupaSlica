cmake_minimum_required(VERSION 3.5.0)
project(ZupaSlica VERSION 0.1.0 LANGUAGES C CXX)


link_directories(/usr/local/lib)
link_directories(/home/xandervaes/dev/assimp/bin)

set(LIBS glfw assimp)


add_executable(ZupaSlica src/main.cpp src/glad.c)


target_include_directories(ZupaSlica PRIVATE ${ZupaSlica_SOURCE_DIR}/include)

target_link_libraries(ZupaSlica ${LIBS})