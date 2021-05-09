﻿#include <iostream>
#include <windows.h>


HWND window;
HINSTANCE windowInstance;
std::string title = "Vulkan Example";
std::string name = "vulkanExample";

HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
{
	bool fullscreen = false;
	int width = 1280;
	int height = 720;

	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndproc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = name.c_str();
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = width;
			dmScreenSettings.dmPelsHeight = height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = false;
				}
				else
				{
					return nullptr;
				}
			}
			screenWidth = width;
			screenHeight = height;
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)width;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = "Example";
	window = CreateWindowEx(0,
		name.c_str(),
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!window)
	{
		printf("Could not create window!\n");
		fflush(stdout);
		return nullptr;
	}

	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	return window;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
//{
//	setupWindow(hInstance, WndProc);
//
//	MSG msg;
//	bool quitMessageReceived = false;
//	while (!quitMessageReceived) {
//		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//			if (msg.message == WM_QUIT) {
//				quitMessageReceived = true;
//				break;
//			}
//		}
//	}
//}

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


struct Vertex
{
	int x;
	int y;
};

bool edgeFunction(const Vertex& a, const Vertex& b, const Vertex& c)
{
	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0);
}

void rasterizeTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, SoftRenderer::FrameBuffer& image, glm::vec4 color)
{
	int min_x = std::min(v0.x, std::min(v1.x, v2.x));
	int min_y = std::min(v0.y, std::min(v1.y, v2.y));
	int max_x = std::max(v0.x, std::max(v1.x, v2.x));
	int max_y = std::max(v0.y, std::max(v1.y, v2.y));

	for (int x = min_x; x <= max_x; ++x)
	{
		for (int y = min_y; y <= max_y; ++y)
		{
			Vertex v = {x, y};
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
void main()
{
	//setupWindow(nullptr, WndProc);
	SoftRenderer::Window* window = SoftRenderer::Window::create("hello", 500, 500);

	SoftRenderer::FrameBuffer frameBuffer(500, 500);
	frameBuffer.clearColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	//line(100, 100, 300, 300, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

	//DrawBresenhamline(10, 10, 400, 100, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

	//DrawBresenhamline2(100, 250, 400, 250, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
	//DrawBresenhamline2(250, 100, 250, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
	//DrawBresenhamline2(100, 400, 400, 100, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
	//DrawBresenhamline2(100, 100, 400, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

	//DrawBresenhamline2(100, 300, 400, 200, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));
	//DrawBresenhamline2(150, 100, 350, 400, frameBuffer, glm::vec4(0.0, 1.0, 0.0, 1.0));

	Vertex v0 = {200, 200};
	Vertex v1 = {250, 300};
	Vertex v2 = {300, 200};

	Vertex v3 = { 250, 100 };
	Vertex v4 = { 200, 200 };
	Vertex v5 = { 300, 200 };


	rasterizeTriangle(v0, v1, v2, frameBuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));
	rasterizeTriangle(v3, v4, v5, frameBuffer, glm::vec4(1.0, 1.0, 0.0, 1.0));

	while (!window->should_close())
	{
		window->drawBuffer(frameBuffer);

		MSG message;
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}



}