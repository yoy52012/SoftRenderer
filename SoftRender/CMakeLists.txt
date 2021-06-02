# CMakeList.txt: SoftRender 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


# 将源代码添加到此项目的可执行文件。
add_executable (SoftRender  "SoftRender.cpp" "SoftRender.h" "Window.h" "Window.cpp" "FrameBuffer.h" "FrameBuffer.cpp" "Mesh.h" "Mesh.cpp" "Pipeline.h" "Pipeline.cpp" "MathUtils.h" "MathUtils.cpp" "Camera.h" "Camera.cpp" "Render.h" "Render.cpp" "Graphics.h" "Graphics.cpp" "Shader.h" "Shader.cpp" "Program.h" "Program.cpp" "Texture.h" "Texture.cpp" "Image.h" "Image.cpp" "Utils.h")

# TODO: 如有需要，请添加测试并安装目标。
