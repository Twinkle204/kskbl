cmake_minimum_required(VERSION 3.20)
project(kskbl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC})

# 将 SDL2 解压到 external/SDL2、GLM 解压到 external/GLM 后，取消注释并核对路径：
# target_include_directories(${PROJECT_NAME} PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/external/GLM"
# )
# target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/lib/x64")
# target_link_libraries(${PROJECT_NAME} PRIVATE SDL2main SDL2 opengl32)
