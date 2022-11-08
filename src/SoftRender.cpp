#include <iostream>
//#include <windows.h>

#include "FrameBuffer.h"
using namespace SoftRenderer;

void line(int x0, int y0, int x1, int y1, SoftRenderer::FrameBuffer& image, glm::vec4 color) {
    for (float t = 0.; t < 1.0; t += 0.01) {
        int x = x0 + (x1 - x0) * t;
        int y = y0 + (y1 - y0) * t;
        image.writeColor(x, y, color);
        std::cout << x << "," << y << std::endl;
    }
}

void DrawBresenhamline(int x0, int y0, int x1, int y1, SoftRenderer::FrameBuffer& image, glm::vec4 color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = 1;
    int sy = 1;
    if (dx < 0)
    {
        sx = -1;
        dx = -dx;
    }

    if (dy < 0)
    {
        sy = -1;
        dy = -dy;
    }


    int dx2 = dx << 1;
    int dy2 = dy << 1;
    int dy2_dx2 = dy2 - dx2;
    int x = x0;
    int y = y0;
    
    if (std::abs(dx) > std::abs(dy))
    {
        int e = dy2 - dx; 
        for(int i = 0; i <= dx; ++i)
        {
            image.writeColor(x, y, color);
            if (e > 0)
            {
                y += sy;
                e += dy2_dx2;
            }
            else 
            {
                e += dy2;
            }
            x += sx;
        }
    }
    else
    {
        int e = dx2 - dy; 
        for(int i = 0; i <= dy; ++i)
        {
            image.writeColor(x, y, color);
            if (e > 0)
            {
                x += sx;
                e -= dy2_dx2;
            }
            else {
                e += dx2;
            }
            y += sy;
        }
    }
}

void DrawBresenhamline2(int x0, int y0, int x1, int y1, SoftRenderer::FrameBuffer& image, glm::vec4 color)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x1 - x0 >= 0 ? 1 : -1;
    int sy = y1 - y0 >= 0 ? 1 : -1;

    bool st = dy > dx;
    if (st)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(dx, dy);
    }

    int dx2 = dx << 1;
    int dy2 = dy << 1;
    int dy2_dx2 = dy2 - dx2;
    int x = x0;
    int y = y0;

    int e = dy2 - dx;
    for (int i = 0; i <= dx; ++i)
    {
        if (st) {
            image.writeColor(y, x, color);
        }
        else {
            image.writeColor(x, y, color);
        }

        if (e > 0)
        {
            y += sy;
            e += dy2_dx2;
        }
        else
        {
            e += dy2;
        }
        x += sx;
    }
}




bool edgeFunction(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c)
{
    return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0);
}

void rasterizeTriangle(const glm::ivec2& v0, const glm::ivec2& v1, const glm::ivec2& v2, SoftRenderer::FrameBuffer& image, glm::vec4 color)
{
    int min_x = std::min(v0.x, std::min(v1.x, v2.x));
    int min_y = std::min(v0.y, std::min(v1.y, v2.y));
    int max_x = std::max(v0.x, std::max(v1.x, v2.x));
    int max_y = std::max(v0.y, std::max(v1.y, v2.y));

    for (int x = min_x; x <= max_x; ++x)
    {
        for (int y = min_y; y <= max_y; ++y)
        {
            glm::vec2 v = {x, y};
            bool insied = true;
            insied &= edgeFunction(v0, v1, v);
            insied &= edgeFunction(v1, v2, v);
            insied &= edgeFunction(v2, v0, v);
            if (insied)
            {
                image.writeColor(x, y, color);
            }
        }
    }
}


//void DrawTriangle1(int x1, int y1, int x2, int y2, int x3, int y3, DWORD color) // 画实心平底三角形
//{
//	for (int y = y1; y <= y2; ++y)
//	{
//		int xs, xe;
//		xs = (y - y1) * (x2 - x1) / (y2 - y1) + x1 + 0.5;
//		xe = (y - y1) * (x3 - x1) / (y3 - y1) + x1 + 0.5;
//		DrawLine(xs, y, xe, y, color);
//	}
//}
//
//void DrawTriangle2(int x1, int y1, int x2, int y2, int x3, int y3, DWORD color) // 画实心平顶三角形
//{
//	for (int y = y1; y <= y3; ++y)
//	{
//		int xs, xe;
//		xs = (y - y1) * (x3 - x1) / (y3 - y1) + x1 + 0.5;
//		xe = (y - y2) * (x3 - x2) / (y3 - y2) + x2 + 0.5;
//		DrawLine(xs, y, xe, y, color);
//	}
//}
//
//void DrawTriangle3(int x1, int y1, int x2, int y2, int x3, int y3, DWORD color) // 画任意实心三角形
//{
//	if (y1 == y2)
//	{
//		if (y3 <= y1) // 平底
//		{
//			DrawTriangle1(x3, y3, x1, y1, x2, y2, color);
//		}
//		else // 平顶
//		{
//			DrawTriangle2(x1, y1, x2, y2, x3, y3, color);
//		}
//	}
//	else if (y1 == y3)
//	{
//		if (y2 <= y1) // 平底
//		{
//			DrawTriangle1(x2, y2, x1, y1, x3, y3, color);
//		}
//		else // 平顶
//		{
//			DrawTriangle2(x1, y1, x3, y3, x2, y2, color);
//		}
//	}
//	else if (y2 == y3)
//	{
//		if (y1 <= y2) // 平底
//		{
//			DrawTriangle1(x1, y1, x2, y2, x3, y3, color);
//		}
//		else // 平顶
//		{
//			DrawTriangle2(x2, y2, x3, y3, x1, y1, color);
//		}
//	}
//	else
//	{
//		double xtop, ytop, xmiddle, ymiddle, xbottom, ybottom;
//		if (y1 < y2 && y2 < y3) // y1 y2 y3
//		{
//			xtop = x1;
//			ytop = y1;
//			xmiddle = x2;
//			ymiddle = y2;
//			xbottom = x3;
//			ybottom = y3;
//		}
//		else if (y1 < y3 && y3 < y2) // y1 y3 y2
//		{
//			xtop = x1;
//			ytop = y1;
//			xmiddle = x3;
//			ymiddle = y3;
//			xbottom = x2;
//			ybottom = y2;
//		}
//		else if (y2 < y1 && y1 < y3) // y2 y1 y3
//		{
//			xtop = x2;
//			ytop = y2;
//			xmiddle = x1;
//			ymiddle = y1;
//			xbottom = x3;
//			ybottom = y3;
//		}
//		else if (y2 < y3 && y3 < y1) // y2 y3 y1
//		{
//			xtop = x2;
//			ytop = y2;
//			xmiddle = x3;
//			ymiddle = y3;
//			xbottom = x1;
//			ybottom = y1;
//		}
//		else if (y3 < y1 && y1 < y2) // y3 y1 y2
//		{
//			xtop = x3;
//			ytop = y3;
//			xmiddle = x1;
//			ymiddle = y1;
//			xbottom = x2;
//			ybottom = y2;
//		}
//		else if (y3 < y2 && y2 < y1) // y3 y2 y1
//		{
//			xtop = x3;
//			ytop = y3;
//			xmiddle = x2;
//			ymiddle = y2;
//			xbottom = x1;
//			ybottom = y1;
//		}
//		int xl; // 长边在ymiddle时的x，来决定长边是在左边还是右边
//		xl = (ymiddle - ytop) * (xbottom - xtop) / (ybottom - ytop) + xtop + 0.5;
//
//		if (xl <= xmiddle) // 左三角形
//		{
//			// 画平底
//			DrawTriangle1(xtop, ytop, xl, ymiddle, xmiddle, ymiddle, color);
//
//			// 画平顶
//			DrawTriangle2(xl, ymiddle, xmiddle, ymiddle, xbottom, ybottom, color);
//		}
//		else // 右三角形
//		{
//			// 画平底
//			DrawTriangle1(xtop, ytop, xmiddle, ymiddle, xl, ymiddle, color);
//
//			// 画平顶
//			DrawTriangle2(xmiddle, ymiddle, xl, ymiddle, xbottom, ybottom, color);
//		}
//	}
//}

#include "Window.h"
#include "Mesh.h"
#include "Camera.h"
#include "MathUtils.h"

#include <glm/gtc/matrix_transform.hpp>

glm::vec3 perspectiveDivede(const glm::vec4& clipCoord)
{
    float x = clipCoord.x / clipCoord.w;
    float y = clipCoord.y / clipCoord.w;
    float z = clipCoord.z / clipCoord.w;
    return glm::vec3(x, y, z);
}

glm::vec2 viewportTransform(int width, int height, const glm::vec3& ndcCoord)
{
    float x = (ndcCoord.x + 1.0f) * 0.5f * static_cast<float>(width);
    float y = (ndcCoord.y + 1.0f) * 0.5f * static_cast<float>(height);
    //float z = (ndcCoord.z + 1.0f) * 0.5f;
    return glm::vec2(x, y);
}

#include <array>

SoftRenderer::FrameBuffer frameBuffer(500, 500);

void draw(const std::array<Vertex, 3>& vertices, const glm::mat4& modelMat, const glm::mat4& viewMat, const glm::mat4& projMat)
{
    std::array<glm::vec4, 3> clipCoord;
    std::array<glm::vec3, 3> ndcCoord;
    std::array<glm::vec2, 3> screenCoord;

    for (int i = 0; i < 3; ++i)
    {
        clipCoord[i] = projMat * viewMat * modelMat * glm::vec4(vertices[i].position, 1.0f);
    }

    for (int i = 0; i < 3; ++i)
    {
        ndcCoord[i] = perspectiveDivede(clipCoord[i]);
    }

    for (int i = 0; i < 3; ++i)
    {
        screenCoord[i] = viewportTransform(500, 500, ndcCoord[i]);
    }

    rasterizeTriangle(screenCoord[0], screenCoord[1], screenCoord[2], frameBuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));
}

#include <chrono>

#include "Graphics.h"
#include "Utils.h"
#include "Image.h"
#include "Texture.h"



void keyCallback(int key, int scancode, int action, int mods)
{
    if (action != InputAction::PRESS)
        return;
    switch (key)
    {
    case InputKeyCode::KEY_W:
        std::cout << "W Key" << std::endl;
        break;
    case InputKeyCode::KEY_A:
        std::cout << "A Key" << std::endl;
        break;
    case InputKeyCode::KEY_S:
        std::cout << "S Key" << std::endl;
        break;
    case InputKeyCode::KEY_D:
        std::cout << "D Key" << std::endl;
        break;
    default:
        break;
    }
}

void mouseButtonCallback(int button, int action, int mods)
{
    std::string act = action == InputAction::PRESS ? "down" : "up";
    std::string b;
    switch (button)
    {
    case InputMouseButton::MOUSE_BUTTON_LEFT:
        b = "Mouse Left";
        break;
    case InputMouseButton::MOUSE_BUTTON_RIGHT:
        b = "Mouse Right";
        break;
    case InputMouseButton::MOUSE_BUTTON_MIDDLE:
        b = "Mouse Middle";
        break;
    default:
        break;
    }

    std::cout << b << " is " << act << std::endl;
}

#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

std::chrono::high_resolution_clock::time_point m_lastTimePoint;
int64_t m_deltaTime = 0;
int64_t m_fpsTimeRecorder = 0;
int64_t m_fpsCounter = 0;
unsigned int m_fps = 0;


void main()
{
    //setupWindow(nullptr, WndProc);
    Window* window = Window::create("hello", 500, 500);

    window->setKeyCallback(std::bind(&keyCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    window->setMouseButtonCallback(std::bind(&mouseButtonCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    window->setWindowTitle("SoftRenderer");
    

    //line(100, 100, 300, 300, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

    //DrawBresenhamline(10, 10, 400, 100, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

    DrawBresenhamline2(100, 250, 400, 250, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
    DrawBresenhamline2(250, 100, 250, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
    DrawBresenhamline2(100, 400, 400, 100, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
    DrawBresenhamline2(100, 100, 400, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

    DrawBresenhamline2(100, 300, 400, 200, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
    DrawBresenhamline2(150, 100, 350, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

    //glm::ivec2 v0 = {250, 400};
    //glm::ivec2 v1 = {400, 250};
    //glm::ivec2 v2 = {100, 250};

    //Vertex v3 = { 250, 100 };
    //Vertex v4 = { 200, 200 };
    //Vertex v5 = { 300, 200 };

    //rasterizeTriangle(v0, v1, v2, frameBuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));
    //rasterizeTriangle(v3, v4, v5, frameBuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));

    glm::vec3 position(0.0f, 0.0f, 5.0f);
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    Camera camera(position, target, 1.0);

    Vertex v1, v2, v3;
    v1.position = glm::vec3(-0.5f, -0.5f, 0.0f);
    v2.position = glm::vec3(0.5f, 0.5f, 0.0f);
    v3.position = glm::vec3(-0.5f, 0.5f, 0.0f);

    v1.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    v2.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    v3.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    Vertex v4, v5, v6;
    v4.position = glm::vec3(-0.5f, -0.5f, 0.0f);
    v5.position = glm::vec3(0.5f, -0.5f, 0.0f);
    v6.position = glm::vec3(0.5f, 0.5f, 0.0f);

    v4.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    v5.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    v6.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    std::shared_ptr<Mesh> plane = Mesh::createPlane(2, 2, 1, 1);
    std::shared_ptr<Mesh> box = Mesh::createBox(2, 2, 2, 1, 1, 1);
    std::shared_ptr<Mesh> sphere = Mesh::createSphere(1.0f, 0.0f, 2.0f * Math::PI, 0.0f, Math::PI);

    //Mesh mesh;
    //mesh.addTriangle(v1, v2, v3);
    //mesh.addTriangle(v4, v5, v6);

    glm::mat4 modelMat = glm::mat4(1.0f);

    auto start = std::chrono::steady_clock::now();

    float count = 0.0f;

    Graphics render;
    render.init(500, 500);

    std::string a = "Cube_BaseColor.png";
    Image::Ptr image = Image::create(IMAGE_DIR + a);

    auto texture = std::make_shared<Texture>();
    texture->initFromImage(image);

    std::shared_ptr<Program> program = std::make_shared<Program>();

    std::shared_ptr<BaseVertexShader> vertexShader = std::make_shared<BaseVertexShader>();
    std::shared_ptr<BaseFragmentShader> fragmentShader = std::make_shared<BaseFragmentShader>();

    std::shared_ptr<BaseShaderUniforms> uniforms = std::make_shared<BaseShaderUniforms>();

    uniforms->uAlbedoMap.bindTexture(texture.get());

    program->vertexShader = vertexShader;
    program->fragmentShader = fragmentShader;

    program->uniforms = uniforms;

    program->link();

    render.useProgram(program);

    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));

    while (!window->should_close())
    {
        render.clearColor(glm::vec4(0.1, 0.1, 0.1, 1.0));
        render.clearDepth(0.0f);

        //render.setModelMatrix(modelMat);
        //render.setViewMatrix(camera.getViewMatrix());
        //render.setProjMatrix(camera.getProjMatrix());

        float degree = std::sinf(count) * 180.0f * 0.1;
        count += 0.001f;
        modelMat = glm::rotate(glm::mat4(1.0f), degree, glm::vec3(0.0f, 1.0f, 0.0f));


        uniforms->uModelMatrix = modelMat;
        uniforms->uModelViewProjectMatrix = camera.getProjMatrix() * camera.getViewMatrix() * modelMat;
        uniforms->uInverseTransposeModelMatrix = glm::mat3(glm::transpose(glm::inverse(modelMat)));
        uniforms->uCameraPostion = glm::vec3(0);
            
        //render.drawTriangle(v1, v2, v3);
        //render.drawTriangle(v4, v5, v6);
        render.drawMesh1(plane.get());

        //render.drawLine(glm::vec2(100, 10), glm::vec2(400, 40));

        //render.drawLine(glm::vec2(100, 100), glm::vec2(400, 400));

        //render.drawLine(glm::vec2(100, 180), glm::vec2(245, 440));

        render.swapBuffer();

        //draw(vertices, modelMat, camera.getViewMatrix(), camera.getProjMatrix());

        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration<double>(now - start).count();
        //std::cout << time << std::endl;


        //modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, +0.005f));

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