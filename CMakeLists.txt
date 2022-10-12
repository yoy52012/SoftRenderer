cmake_minimum_required (VERSION 3.10)
project ("SoftRenderer")

set(CMAKE_CXX_STANDARD 11)
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

include_directories(
        "${THIRD_PARTY_DIR}/glm"
        "${THIRD_PARTY_DIR}/assimp/include"
        "${THIRD_PARTY_DIR}/assimp/contrib"
        "${CMAKE_CURRENT_BINARY_DIR}/third_party/assimp/include"
        "${THIRD_PARTY_DIR}/stb"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_definitions(-DRESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resource")

# assimp
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
#set(ASSIMP_BUIlD_ASSIMP_VIEW OFF)
set(ASSIMP_BUILD_ZLIB ON) 

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE)
set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)

add_subdirectory(${THIRD_PARTY_DIR}/assimp)

# main src
file(GLOB SOFTRENDERER_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MathUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Program.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Render.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SoftRender.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp
        )


add_executable(SoftRenderer
        "${SOFTRENDERER_SRC}"
        )


if (WIN32)
    if (MSVC)
        target_link_libraries(SoftRenderer
                assimp
                )
    else ()
        target_link_libraries(SoftRenderer
                assimp
                )
    endif ()
endif ()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    target_link_libraries(SoftRenderer
            assimp
            )
endif ()


# output dir
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)


