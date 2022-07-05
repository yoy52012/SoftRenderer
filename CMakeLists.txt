# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("SoftRender")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm-master)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb-master)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader)

add_definitions(-DRESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resource")


# 包含子项目。
add_subdirectory ("SoftRender")
