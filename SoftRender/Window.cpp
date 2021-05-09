#include "Window.h"

#include <iostream>

#include <cassert>
#include <cstring>

#define _CRT_SECURE_NO_WARNINGS

namespace SoftRenderer
{

#ifdef UNICODE
    static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
    static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
#else
    static const char* const WINDOW_CLASS_NAME = "Class";
    static const char* const WINDOW_ENTRY_NAME = "Entry";
#endif

#define LINE_SIZE 256

    Window* Window::create(const std::string& title, int width, int height)
    {
        Window* window = new Window(title, width, height);
        window->init(title, width, height);
        return window;
    }

    Window::Window(const std::string& title, int width, int height)
        :m_width(width), m_height(height), m_frame_data(nullptr)
    {
        assert(width > 0 && height > 0);
    }

    Window::~Window()
    {
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return (DefWindowProc(hWnd, uMsg, wParam, lParam));
    }

    void Window::init(const std::string& title, int width, int height)
    {
        HWND handle;
        HDC memory_dc;

        handle =  createWindow(title, width, height);
        createSurface(handle, width, height, &memory_dc);

        m_handle = handle;
        m_memory_dc = memory_dc;

        SetProp(handle, WINDOW_ENTRY_NAME, this);

        ShowWindow(handle, SW_SHOW);
    }

    void Window::destroy()
    {
        ShowWindow(m_handle, SW_HIDE);
        RemoveProp(m_handle, WINDOW_ENTRY_NAME);

        DeleteDC(m_memory_dc);
        DestroyWindow(m_handle);

        m_frame_data = nullptr;
        //free(m_frame_data);
    }

    bool Window::should_close()
    {
        return m_should_close;
    }

    void Window::drawBuffer(FrameBuffer& buffer)
    {
        auto private_blit_bgr = [](FrameBuffer& src, unsigned char* dst) {
            int width = src.getWidth();
            int height = src.getHeight();
            int r, c;

            //assert(src->width == dst->width && src->height == dst->height);
            //assert(dst->format == FORMAT_LDR && dst->channels == 4);

            for (r = 0; r < height; r++) {
                for (c = 0; c < width; c++) {
                    int flipped_r = height - 1 - r;
                    int src_index = (r * width + c) * 4;
                    int dst_index = (flipped_r * width + c) * 4;

                    unsigned char* src_pixel = &(src.getColorBuffer()[src_index]);

                    //unsigned char* src_pixel = src.getColorBuffer();//&src.[src_index];
                    unsigned char* dst_pixel = &dst[dst_index];
                    dst_pixel[0] = src_pixel[2];  /* blue */
                    dst_pixel[1] = src_pixel[1];  /* green */
                    dst_pixel[2] = src_pixel[0];  /* red */
                }
            }
        };

        private_blit_bgr(buffer, m_frame_data);

        presentSurface();
    }


    HWND Window::createWindow(const std::string& title, int width, int height)
    {
        // Registe Class 
        WNDCLASS window_class;
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = WndProc;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.hInstance = GetModuleHandle(NULL);
        window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        window_class.lpszMenuName = NULL;
        window_class.lpszClassName = WINDOW_CLASS_NAME;

        if (!RegisterClass(&window_class))
        {
            std::cerr << "Could not register window class!\n";
            fflush(stdout);
            exit(1);
        }
        
        HWND handle;

        DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        AdjustWindowRect(&rect, style, FALSE);
        
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        handle = CreateWindow(WINDOW_CLASS_NAME, title.c_str(), style,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        assert(handle != nullptr);

        return handle;
    }

    void Window::createSurface(HWND handle, int width, int height, HDC* out_memory_dc)
    {
        BITMAPINFOHEADER bi_header;
        HBITMAP dib_bitmap;
        HBITMAP old_bitmap;
        HDC window_dc;
        HDC memory_dc;

        window_dc = GetDC(handle);
        memory_dc = CreateCompatibleDC(window_dc);
        ReleaseDC(handle, window_dc);

        memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
        bi_header.biSize = sizeof(BITMAPINFOHEADER);
        bi_header.biWidth = width;
        bi_header.biHeight = -height;  /* top-down */
        bi_header.biPlanes = 1;
        bi_header.biBitCount = 32;
        bi_header.biCompression = BI_RGB;
        dib_bitmap = CreateDIBSection(memory_dc, (BITMAPINFO*)&bi_header,
            DIB_RGB_COLORS, (void**)&m_frame_data,
            nullptr, 0);
        assert(dib_bitmap != nullptr);
        old_bitmap = (HBITMAP)SelectObject(memory_dc, dib_bitmap);
        DeleteObject(old_bitmap);

        *out_memory_dc = memory_dc;
    }

    void Window::presentSurface()
    {
        HDC window_dc = GetDC(m_handle);
        HDC memory_dc = m_memory_dc;
        //auto surface = window->surface;
        int width = m_width;
        int height = m_height;
        BitBlt(window_dc, 0, 0, width, height, memory_dc, 0, 0, SRCCOPY);
        ReleaseDC(m_handle, window_dc);
    }

}