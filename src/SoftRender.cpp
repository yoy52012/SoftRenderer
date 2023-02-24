#include <chrono>
#include <array>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "FrameBuffer.h"
#include "Mesh.h"
#include "Camera.h"
#include "MathUtils.h"
#include "Graphics.h"
#include "Utils.h"
#include "Image.h"
#include "Texture.h"
#include "OrbitControls.h"
#include "SceneLoader.h"
#include "ShaderManagement.h"
#include "Material.h"

using namespace SoftRenderer;

SoftRenderer::FrameBuffer frameBuffer(500, 500);

std::chrono::high_resolution_clock::time_point m_lastTimePoint;
int64_t m_deltaTime = 0;
int64_t m_fpsTimeRecorder = 0;
int64_t m_fpsCounter = 0;
unsigned int m_fps = 0;

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};

void main()
{
    ShaderManager::instance().init();
    BlinnPhongMaterial material;
    material.setShaderProgram(ShaderManager::instance().getShaderProgram("BlinnPhong"));

    Window* window = Window::create("hello", 500, 500);

    window->setWindowTitle("SoftRenderer");

    glm::vec3 position(0.0f, 0.0f, 3.0f);
    glm::vec3 target(0.0f, 1.0f, 0.0f);
    Camera camera(60.0f, (float)500 / (float)500, 0.1f, 100.0f);
    camera.lookAt(position, target);

    OrbitControllers orbitControllers(camera, window);

    std::shared_ptr<Mesh> plane = Mesh::createPlane(2, 2, 1, 1);
    std::shared_ptr<Mesh> box = Mesh::createBox(2, 2, 2, 1, 1, 1);
    std::shared_ptr<Mesh> sphere = Mesh::createSphere(1.0f, 0.0f, 2.0f * Math::PI, 0.0f, Math::PI);
    std::shared_ptr<Mesh> torusKnot = Mesh::createTorusKnot(10, 3, 64, 8, 2, 3);

    std::shared_ptr<Mesh> model = std::make_shared<Mesh>();
    SceneLoader::instance().loadModel("E:/OpenProject/SoftGLRender/assets/DamagedHelmet/DamagedHelmet.gltf", model);


    std::vector<uint32_t> indices;
    std::vector<glm::vec3> positions;

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) 
        {
            glm::vec3 position;
            position.x = skyboxVertices[i * 9 + j * 3 + 0];
            position.y = skyboxVertices[i * 9 + j * 3 + 1];
            position.z = skyboxVertices[i * 9 + j * 3 + 2];
            positions.push_back(position);
            indices.push_back(i * 3 + j);
        }
    }
    
    std::shared_ptr<SubMesh> submesh = std::make_shared<SubMesh>();
    submesh->setPositions(positions);
    submesh->setIndices(indices);
    submesh->build();

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->addSubMesh(submesh);


    glm::mat4 modelMat = glm::mat4(1.0f);

    auto start = std::chrono::steady_clock::now();

    Graphics& render = Graphics::instance();
    render.init(500, 500);

    std::string a = "Default_albedo.jpg";
    Image::Ptr image = Image::create(IMAGE_DIR + a);

    auto texture = std::make_shared<Texture>();
    texture->initFromImage(image);

    material.setDiffuseColor(glm::vec3(1.0f, 1.0f, 1.0f));
    material.setSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
    material.setSpecularShininess(30.0f);
    material.setSpecularStrength(1.0f);
    material.setDiffuseTexture(texture);

    auto light_position_angle = 0.0f;


    std::string right  = "Lake/right.jpg";
    std::string left   = "Lake/left.jpg";
    std::string top    = "Lake/top.jpg";
    std::string bottom = "Lake/bottom.jpg";
    std::string front  = "Lake/front.jpg";
    std::string back   = "Lake/back.jpg";

    Image::Ptr rightImage = Image::create(IMAGE_DIR + right);
    Image::Ptr leftImage = Image::create(IMAGE_DIR + left);
    Image::Ptr topImage = Image::create(IMAGE_DIR + top);
    Image::Ptr bottomImage = Image::create(IMAGE_DIR + bottom);
    Image::Ptr frontImage = Image::create(IMAGE_DIR + front);
    Image::Ptr backImage = Image::create(IMAGE_DIR + back);

    auto rightTexture = std::make_shared<Texture>();
    auto leftTexture = std::make_shared<Texture>();
    auto topTexture = std::make_shared<Texture>();
    auto bottomTexture = std::make_shared<Texture>();
    auto frontTexture = std::make_shared<Texture>();
    auto backTexture = std::make_shared<Texture>();

    rightTexture->initFromImage(rightImage);
    leftTexture->initFromImage(leftImage);
    topTexture->initFromImage(topImage);
    bottomTexture->initFromImage(bottomImage);
    frontTexture->initFromImage(frontImage);
    backTexture->initFromImage(backImage);

    std::shared_ptr<Program> skyboxProgram = ShaderManager::instance().getShaderProgram("Skybox");
    std::shared_ptr<SkyboxShaderUniforms> skyboxUniforms = std::dynamic_pointer_cast<SkyboxShaderUniforms>(skyboxProgram->uniforms);
    skyboxUniforms->cubeMap.bindTexture(rightTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_X);
    skyboxUniforms->cubeMap.bindTexture(leftTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_X);
    skyboxUniforms->cubeMap.bindTexture(topTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_Y);
    skyboxUniforms->cubeMap.bindTexture(bottomTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_Y);
    skyboxUniforms->cubeMap.bindTexture(frontTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_POSITIVE_Z);
    skyboxUniforms->cubeMap.bindTexture(backTexture.get(), CubeMapFace::TEXTURE_CUBE_MAP_NEGATIVE_Z);



    Graphics::instance().useProgram(skyboxProgram);


    while (!window->shouldClose())
    {
        render.setViewport(0, 0, 500, 500);
        render.clearColor(glm::vec4(0.1, 0.1, 0.1, 1.0));
        render.clearDepth(0.0f);

        auto light_position = 2.f * glm::vec3(glm::sin(light_position_angle),
                                    0.0f,
                                    glm::cos(light_position_angle));

        //material.bind();
        //material.setModelMatrix(modelMat);
        //material.setModelViewProjectMatrix(camera.getProjMatrix() * camera.getViewMatrix() * modelMat);
        //material.setInverseTransposeModelMatrix(glm::mat3(glm::transpose(glm::inverse(modelMat))));
        //material.setLightPosition(light_position);
        //material.setLightColor(glm::vec3(1.0f, 0.0f, 0.0f));
        //material.setCameraPosition(camera.getEye());
        //material.updateParameters();
        //render.drawMesh1(box.get());



        skyboxUniforms->modelMatrix = glm::mat4(1.0f);
        glm::mat4 view_matrix = glm::mat3(camera.getViewMatrix());
        skyboxUniforms->modelViewProjectMatrix = camera.getProjMatrix() * view_matrix * glm::mat4(1.0f);
        render.drawMesh1(mesh.get());



        render.swapBuffer();

        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration<double>(now - start).count();
        //std::cout << time << std::endl;

        window->drawBuffer(render.getOutput());

        window->pollEvent();

        m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_lastTimePoint).count();
        m_lastTimePoint = std::chrono::high_resolution_clock::now();

        //FPS counting
        {
            m_fpsTimeRecorder += m_deltaTime;
            ++m_fpsCounter;
            if (m_fpsTimeRecorder > 1000)
            {
                m_fps = static_cast<unsigned int>(m_fpsCounter);
                m_fpsCounter = 0.0f;
                m_fpsTimeRecorder = 0.0f;

                std::stringstream ss;
                ss << " FPS:" << std::setiosflags(std::ios::left) << std::setw(3) << m_fps;

                std::cout << ss.str() << std::endl;
            }
        }
    }

}