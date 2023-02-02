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
    std::shared_ptr<Mesh> box = Mesh::createBox(1, 1, 1, 1, 1, 1);
    std::shared_ptr<Mesh> sphere = Mesh::createSphere(1.0f, 0.0f, 2.0f * Math::PI, 0.0f, Math::PI);
    std::shared_ptr<Mesh> torusKnot = Mesh::createTorusKnot(10, 3, 64, 8, 2, 3);

    std::shared_ptr<Mesh> model = std::make_shared<Mesh>();
    SceneLoader::instance().loadModel("E:/OpenProject/SoftGLRender/assets/DamagedHelmet/DamagedHelmet.gltf", model);


    glm::mat4 modelMat = glm::mat4(1.0f);

    auto start = std::chrono::steady_clock::now();

    float count = 0.0f;

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

    //std::shared_ptr<Program> program = std::make_shared<Program>();

    //std::shared_ptr<BaseVertexShader> vertexShader = std::make_shared<BaseVertexShader>();
    //std::shared_ptr<BaseFragmentShader> fragmentShader = std::make_shared<BaseFragmentShader>();

    //std::shared_ptr<BaseShaderUniforms> uniforms = std::make_shared<BaseShaderUniforms>();

    //uniforms->uAlbedoMap.bindTexture(texture.get());

    //program->vertexShader = vertexShader;
    //program->fragmentShader = fragmentShader;

    //program->uniforms = uniforms;

    //program->link();

    //render.useProgram(program);

    auto light_position_angle = 0.0f;


    while (!window->shouldClose())
    {
        render.setViewport(0, 0, 500, 500);
        render.clearColor(glm::vec4(0.1, 0.1, 0.1, 1.0));
        render.clearDepth(0.0f);


        

        //float degree = std::sinf(count) * 180.0f * 0.1;
        //count += 0.001f;
        //modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, degree));
        //modelMat = glm::rotate(glm::mat4(1.0f), degree, glm::vec3(0.0f, 1.0f, 0.0f));


        //light_position_angle += 0.1;
        auto light_position = 2.f * glm::vec3(glm::sin(light_position_angle),
                                    0.0f,
                                    glm::cos(light_position_angle));

        material.bind();

        material.setModelMatrix(modelMat);
        material.setModelViewProjectMatrix(camera.getProjMatrix() * camera.getViewMatrix() * modelMat);
        material.setInverseTransposeModelMatrix(glm::mat3(glm::transpose(glm::inverse(modelMat))));
        material.setLightPosition(light_position);
        material.setLightColor(glm::vec3(1.0f, 0.0f, 0.0f));
        material.setCameraPosition(camera.getEye());
        
        material.updateParameters();

        //uniforms->modelMatrix = modelMat;
        //uniforms->modelViewProjectMatrix = camera.getProjMatrix() * camera.getViewMatrix() * modelMat;
        //uniforms->inverseTransposeModelMatrix = glm::mat3(glm::transpose(glm::inverse(modelMat)));
        //uniforms->cameraPostion = glm::vec3(0);

        render.drawMesh1(model.get());

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