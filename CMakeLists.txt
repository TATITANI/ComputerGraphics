cmake_minimum_required(VERSION 3.14)

set(PROEJCT_NAME ComputerGraphics)
set(CMAKE_CXX_STANDARD 17)

set(WINDOW_NAME "Hello, OpenGL!")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)


project(${PROEJCT_NAME})
add_executable(${PROEJCT_NAME} 
src/main.cpp
src/common.cpp src/common.h
src/shader.cpp src/shader.h
src/program.cpp src/program.h
src/context.cpp src/context.h
src/buffer.cpp src/buffer.h
src/vertexLayout.cpp src/vertexLayout.h
src/image.cpp src/image.h
src/texture.cpp src/texture.h
src/mesh.cpp src/mesh.h
src/model.cpp src/model.h
src/framebuffer.cpp src/framebuffer.h
src/object.cpp src/object.h
)


# ExternalProject 관련 명령어 셋 추가
include(Dependency.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

target_compile_definitions(${PROJECT_NAME} PUBLIC
  WINDOW_NAME="${WINDOW_NAME}"
  WINDOW_WIDTH=${WINDOW_WIDTH}
  WINDOW_HEIGHT=${WINDOW_HEIGHT}
  )


# Dependency들이 먼저 build 될 수 있게 관계 설정
add_dependencies(${PROJECT_NAME} ${DEP_LIST})




 
