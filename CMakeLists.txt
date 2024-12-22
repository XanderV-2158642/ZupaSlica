cmake_minimum_required(VERSION 3.5.0)
project(ZupaSlica VERSION 0.1.0 LANGUAGES C CXX)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

link_directories(/usr/local/lib)
link_directories(/home/xandervaes/dev/assimp/bin)




find_package(OpenMP)
if(OpenMP_CXX_FOUND)
set(LIBS PUBLIC glfw assimp OpenMP::OpenMP_CXX)
endif()


set(IMGUI include/imgui/imgui.cpp
           include/imgui/imgui_demo.cpp
           include/imgui/imgui_draw.cpp
           include/imgui/imgui_impl_glfw.cpp
           include/imgui/imgui_impl_opengl3.cpp
           include/imgui/imgui_widgets.cpp
           include/imgui/imgui_tables.cpp
           include/imgui/imconfig.h
           include/imgui/imgui_impl_glfw.h
           include/imgui/imgui_impl_opengl3_loader.h
           include/imgui/imgui_impl_opengl3.h
           include/imgui/imgui_internal.h
           include/imgui/imgui.h
           include/imgui/imstb_rectpack.h
           include/imgui/imstb_textedit.h
           include/imgui/imstb_truetype.h)

set(CLIPPER include/clipper2/src/clipper.engine.cpp
            include/clipper2/src/clipper.offset.cpp
            include/clipper2/src/clipper.rectclip.cpp)


add_executable(ZupaSlica src/main.cpp src/libs/glad.c ${IMGUI} src/FrameBuffer/FrameBuffer.cpp ${CLIPPER})


target_include_directories(ZupaSlica PRIVATE ${ZupaSlica_SOURCE_DIR}/include)

target_link_libraries(ZupaSlica ${LIBS})
