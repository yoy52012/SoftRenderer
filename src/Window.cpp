#include "Window.h"

#include <iostream>

#include <cassert>
#include <cstring>

#include <windowsx.h>

#include "Input.h"

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

    // Returns the window style for the specified window
    static DWORD getWindowStyle(const Window* window)
    {
        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        style |= WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_MAXIMIZEBOX | WS_THICKFRAME;
        return style;
    }

    // Returns the extended window style for the specified window
    static DWORD getWindowExStyle(const Window* window)
    {
        DWORD style = WS_EX_APPWINDOW;
        return style;
    }

    static int getKeyMods(void)
    {
        int mods = 0;

        if (GetKeyState(VK_SHIFT) & 0x8000)
            mods |= KeyMod::KEY_MOD_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000)
            mods |= KeyMod::KEY_MOD_CONTROL;
        if (GetKeyState(VK_MENU) & 0x8000)
            mods |= KeyMod::KEY_MOD_ALT;
        if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
            mods |= KeyMod::KEY_MOD_SUPER;
        if (GetKeyState(VK_CAPITAL) & 1)
            mods |= KeyMod::KEY_MOD_CAPS_LOCK;
        if (GetKeyState(VK_NUMLOCK) & 1)
            mods |= KeyMod::KEY_MOD_NUM_LOCK;

        return mods;
    }

    LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Window* window = (Window*)GetProp(hWnd, WINDOW_ENTRY_NAME);

        if (window == nullptr) {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        switch (uMsg)
        {
        case WM_CLOSE:
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            int key, scancode = -1;
            const int action = (HIWORD(lParam) & KF_UP) ? InputAction::RELEASE : InputAction::PRESS;
            const int mods = getKeyMods();

            scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
            if (!scancode)
            {
                // NOTE: Some synthetic key messages have a scancode of zero
                // HACK: Map the virtual key back to a usable scancode
                scancode = MapVirtualKeyW((UINT)wParam, MAPVK_VK_TO_VSC);
            }

            if (window->m_keycode_map.find(scancode) != window->m_keycode_map.end())
            {
                key = window->m_keycode_map[scancode];
            }

            // The Ctrl keys require special handling
            if (wParam == VK_CONTROL)
            {
                if (HIWORD(lParam) & KF_EXTENDED)
                {
                    // Right side keys have the extended key bit set
                    key = InputKeyCode::KEY_RIGHT_CONTROL;
                }
                else
                {
                    key = InputKeyCode::KEY_LEFT_CONTROL;
                }
            }
            else if (wParam == VK_PROCESSKEY)
            {
                // IME notifies that keys have been filtered by setting the
                // virtual key-code to VK_PROCESSKEY
                break;
            }

            window->inputKeyNotify(key, scancode, action, mods);

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int i, button, action;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
                button = InputMouseButton::MOUSE_BUTTON_LEFT;
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
                button = InputMouseButton::MOUSE_BUTTON_RIGHT;
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
                button = InputMouseButton::MOUSE_BUTTON_MIDDLE;
            
            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
                uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)
            {
                action = InputAction::PRESS;
            }
            else
            {
                action = InputAction::RELEASE;
            }

            for (i = 0; i <= InputMouseButton::MOUSE_BUTTON_LAST; i++)
            {
                if (window->m_mousebutton_action[i] == InputAction::PRESS)
                    break;
            }

            if (i > InputMouseButton::MOUSE_BUTTON_LAST)
            {
                SetCapture(hWnd);
            }

            window->inputMouseClickNotify(button, action, getKeyMods());

            for (i = 0; i <= InputMouseButton::MOUSE_BUTTON_LAST; i++)
            {
                if (window->m_mousebutton_action[i] == InputAction::PRESS)
                    break;
            }

            if (i > InputMouseButton::MOUSE_BUTTON_LAST)
            {
                ReleaseCapture();
            }

            if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP)
                return TRUE;

            return 0;
        }
        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            if (!window->mIsCursorTracked)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->m_handle;
                TrackMouseEvent(&tme);

                window->mIsCursorTracked = true;
                window->inputCursorEnterCallback(true);
            }

            window->inputCursorPosCallback(x, y);

            window->mLastCursorPosX = x;
            window->mLastCursorPosY = y;

            return 0;
        }

        default:
            break;
        }

        return (DefWindowProc(hWnd, uMsg, wParam, lParam));
    }

    void Window::init(const std::string& title, int width, int height)
    {
        initKeyMaps();

        HWND handle;
        HDC memory_dc;

        handle =  createWindow(title, width, height);
        createSurface(handle, width, height, &memory_dc);

        m_handle = handle;
        m_memory_dc = memory_dc;

        SetProp(handle, WINDOW_ENTRY_NAME, this);

        ShowWindow(handle, SW_SHOW);
    }

    void Window::initKeyMaps()
    {
        m_keycode_map = 
        {
            {0x00B, InputKeyCode::KEY_0},
            {0x002, InputKeyCode::KEY_1},
            {0x003, InputKeyCode::KEY_2},
            {0x004, InputKeyCode::KEY_3},
            {0x005, InputKeyCode::KEY_4},
            {0x006, InputKeyCode::KEY_5},
            {0x007, InputKeyCode::KEY_6},
            {0x008, InputKeyCode::KEY_7},
            {0x009, InputKeyCode::KEY_8},
            {0x00A, InputKeyCode::KEY_9},
            {0x01E, InputKeyCode::KEY_A},
            {0x030, InputKeyCode::KEY_B},
            {0x02E, InputKeyCode::KEY_C},
            {0x020, InputKeyCode::KEY_D},
            {0x012, InputKeyCode::KEY_E},
            {0x021, InputKeyCode::KEY_F},
            {0x022, InputKeyCode::KEY_G},
            {0x023, InputKeyCode::KEY_H},
            {0x017, InputKeyCode::KEY_I},
            {0x024, InputKeyCode::KEY_J},
            {0x025, InputKeyCode::KEY_K},
            {0x026, InputKeyCode::KEY_L},
            {0x032, InputKeyCode::KEY_M},
            {0x031, InputKeyCode::KEY_N},
            {0x018, InputKeyCode::KEY_O},
            {0x019, InputKeyCode::KEY_P},
            {0x010, InputKeyCode::KEY_Q},
            {0x013, InputKeyCode::KEY_R},
            {0x01F, InputKeyCode::KEY_S},
            {0x014, InputKeyCode::KEY_T},
            {0x016, InputKeyCode::KEY_U},
            {0x02F, InputKeyCode::KEY_V},
            {0x011, InputKeyCode::KEY_W},
            {0x02D, InputKeyCode::KEY_X},
            {0x015, InputKeyCode::KEY_Y},
            {0x02C, InputKeyCode::KEY_Z},

            {0x028, InputKeyCode::KEY_APOSTROPHE},
            {0x02B, InputKeyCode::KEY_BACKSLASH},
            {0x033, InputKeyCode::KEY_COMMA},
            {0x00D, InputKeyCode::KEY_EQUAL},
            {0x029, InputKeyCode::KEY_GRAVE_ACCENT},
            {0x01A, InputKeyCode::KEY_LEFT_BRACKET},
            {0x00C, InputKeyCode::KEY_MINUS},
            {0x034, InputKeyCode::KEY_PERIOD},
            {0x01B, InputKeyCode::KEY_RIGHT_BRACKET},
            {0x027, InputKeyCode::KEY_SEMICOLON},
            {0x035, InputKeyCode::KEY_SLASH},
            {0x056, InputKeyCode::KEY_WORLD_2},

            {0x00E, InputKeyCode::KEY_BACKSPACE},
            {0x153, InputKeyCode::KEY_DELETE},
            {0x14F, InputKeyCode::KEY_END},
            {0x01C, InputKeyCode::KEY_ENTER},
            {0x001, InputKeyCode::KEY_ESCAPE},
            {0x147, InputKeyCode::KEY_HOME},
            {0x152, InputKeyCode::KEY_INSERT},
            {0x15D, InputKeyCode::KEY_MENU},
            {0x151, InputKeyCode::KEY_PAGE_DOWN},
            {0x149, InputKeyCode::KEY_PAGE_UP},
            {0x045, InputKeyCode::KEY_PAUSE},
            {0x146, InputKeyCode::KEY_PAUSE},
            {0x039, InputKeyCode::KEY_SPACE},
            {0x00F, InputKeyCode::KEY_TAB},
            {0x03A, InputKeyCode::KEY_CAPS_LOCK},
            {0x145, InputKeyCode::KEY_NUM_LOCK},
            {0x046, InputKeyCode::KEY_SCROLL_LOCK},
            {0x03B, InputKeyCode::KEY_F1},
            {0x03C, InputKeyCode::KEY_F2},
            {0x03D, InputKeyCode::KEY_F3},
            {0x03E, InputKeyCode::KEY_F4},
            {0x03F, InputKeyCode::KEY_F5},
            {0x040, InputKeyCode::KEY_F6},
            {0x041, InputKeyCode::KEY_F7},
            {0x042, InputKeyCode::KEY_F8},
            {0x043, InputKeyCode::KEY_F9},
            {0x044, InputKeyCode::KEY_F10},
            {0x057, InputKeyCode::KEY_F11},
            {0x058, InputKeyCode::KEY_F12},
            {0x064, InputKeyCode::KEY_F13},
            {0x065, InputKeyCode::KEY_F14},
            {0x066, InputKeyCode::KEY_F15},
            {0x067, InputKeyCode::KEY_F16},
            {0x068, InputKeyCode::KEY_F17},
            {0x069, InputKeyCode::KEY_F18},
            {0x06A, InputKeyCode::KEY_F19},
            {0x06B, InputKeyCode::KEY_F20},
            {0x06C, InputKeyCode::KEY_F21},
            {0x06D, InputKeyCode::KEY_F22},
            {0x06E, InputKeyCode::KEY_F23},
            {0x076, InputKeyCode::KEY_F24},
            {0x038, InputKeyCode::KEY_LEFT_ALT},
            {0x01D, InputKeyCode::KEY_LEFT_CONTROL},
            {0x02A, InputKeyCode::KEY_LEFT_SHIFT},
            {0x15B, InputKeyCode::KEY_LEFT_SUPER},
            {0x137, InputKeyCode::KEY_PRINT_SCREEN},
            {0x138, InputKeyCode::KEY_RIGHT_ALT},
            {0x11D, InputKeyCode::KEY_RIGHT_CONTROL},
            {0x036, InputKeyCode::KEY_RIGHT_SHIFT},
            {0x15C, InputKeyCode::KEY_RIGHT_SUPER},
            {0x150, InputKeyCode::KEY_DOWN},
            {0x14B, InputKeyCode::KEY_LEFT},
            {0x14D, InputKeyCode::KEY_RIGHT},
            {0x148, InputKeyCode::KEY_UP},

            {0x052, InputKeyCode::KEY_KP_0},
            {0x04F, InputKeyCode::KEY_KP_1},
            {0x050, InputKeyCode::KEY_KP_2},
            {0x051, InputKeyCode::KEY_KP_3},
            {0x04B, InputKeyCode::KEY_KP_4},
            {0x04C, InputKeyCode::KEY_KP_5},
            {0x04D, InputKeyCode::KEY_KP_6},
            {0x047, InputKeyCode::KEY_KP_7},
            {0x048, InputKeyCode::KEY_KP_8},
            {0x049, InputKeyCode::KEY_KP_9},
            {0x04E, InputKeyCode::KEY_KP_ADD},
            {0x053, InputKeyCode::KEY_KP_DECIMAL},
            {0x135, InputKeyCode::KEY_KP_DIVIDE},
            {0x11C, InputKeyCode::KEY_KP_ENTER},
            {0x059, InputKeyCode::KEY_KP_EQUAL},
            {0x037, InputKeyCode::KEY_KP_MULTIPLY},
            {0x04A, InputKeyCode::KEY_KP_SUBTRACT},
        };
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

    void Window::drawBuffer(const FrameBuffer::Ptr& buffer)
    {
        auto private_blit_bgr = [](const FrameBuffer::Ptr& src, unsigned char* dst) {
            int width = src->getWidth();
            int height = src->getHeight();
            int r, c;

            //assert(src->width == dst->width && src->height == dst->height);
            //assert(dst->format == FORMAT_LDR && dst->channels == 4);

            for (r = 0; r < height; r++) {
                for (c = 0; c < width; c++) {
                    int flipped_r = height - 1 - r;
                    int src_index = (r * width + c) * 4;
                    int dst_index = (flipped_r * width + c) * 4;

                    unsigned char* src_pixel = &(src->getColorBuffer()[src_index]);

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

    void Window::pollEvent()
    {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }

    void Window::setKeyCallback(KeyCallback keyCallback)
    {
        m_callbacks.key = keyCallback;
    }

    void Window::setMouseButtonCallback(MouseButtonCallback mouseButtonCallback)
    {
        m_callbacks.mousebutton = mouseButtonCallback;
    }

    void Window::setCursorPosCallback(CursorPositionCallback cursorPositionCallback)
    {
        m_callbacks.cursorPos = cursorPositionCallback;
    }

    void Window::setCursorEnterCallback(CursorEnterCallback cursorEnterCallback)
    {
        m_callbacks.cursorEnter = cursorEnterCallback;
    }

    void Window::getCursorPos(double* x, double* y)
    {
        POINT pos;

        if (GetCursorPos(&pos))
        {
            ScreenToClient(m_handle, &pos);

            if (x)
                *x = pos.x;
            if (y)
                *y = pos.y;
        }
    }

    void Window::setCursorPos(double x, double y)
    {
        POINT pos = { (int)x, (int)y };

        ClientToScreen(m_handle, &pos);
        SetCursorPos(pos.x, pos.y);

        mLastCursorPosX = pos.x;
        mLastCursorPosY = pos.y;
    }

    WCHAR* createWideStringFromUTF8Win32(const char* source)
    {
        WCHAR* target;
        int count;

        count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
        if (!count)
        {
            std::cerr << "Win32: Failed to convert string from UTF-8" << std::endl;
            return NULL;
        }

        target = (WCHAR*)std::malloc(count * sizeof(WCHAR));
        std::memset(target, 0, count * sizeof(WCHAR));

        if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
        {
            free(target);
            std::cerr << "Win32: Failed to convert string from UTF-8" << std::endl;
            return NULL;
        }

        return target;
    }

    void Window::setWindowTitle(const std::string& title)
    {
        WCHAR* wideTitle = createWideStringFromUTF8Win32(title.c_str());
        if (!wideTitle)
            return;

        SetWindowTextW(m_handle, wideTitle);
        free(wideTitle);
    }

    void Window::getWindowSize(int* width, int* height)
    {
        RECT area;
        GetClientRect(m_handle, &area);

        if (width)
            *width = area.right;
        if (height)
            *height = area.bottom;
    }

    void Window::setWindowSize(int width, int height)
    {
        assert(width >= 0);
        assert(height >= 0);

        RECT rect = { 0, 0, width, height };
        AdjustWindowRectExForDpi(&rect, getWindowStyle(this), FALSE, getWindowExStyle(this), GetDpiForWindow(m_handle));

        SetWindowPos(m_handle, HWND_TOP,
            0, 0, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }


    HWND Window::createWindow(const std::string& title, int width, int height)
    {
        // https://blog.csdn.net/bizhu12/article/details/6667580       
        // Registe Class 
        WNDCLASS window_class;
        ZeroMemory(&window_class, sizeof(window_class));
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = Window::WndProc;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.hInstance = GetModuleHandle(NULL);
        window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);    
        window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

    void Window::inputKeyNotify(int key, int scancode, int action, int mods)
    {
        if (key >= 0 && key <= InputKeyCode::KEY_LAST)
        {
            bool repeated = false;

            if (action == InputAction::RELEASE && m_key_action[key] == InputAction::RELEASE)
            {
                return;
            }

            if (action == InputAction::PRESS && m_key_action[key] == InputAction::PRESS)
            {
                repeated == true;
            }

            if (repeated)
            {
                action = InputAction::REPEAT;
            }
        }

        if (!m_is_lock_mods)
            mods &= ~(KeyMod::KEY_MOD_NUM_LOCK | KeyMod::KEY_MOD_CAPS_LOCK);

        if (m_callbacks.key)
        {
            m_callbacks.key(key, scancode, action, mods);
        }
    }

    void Window::inputMouseClickNotify(int button, int action, int mods)
    {
        if (button < 0 || button > InputMouseButton::MOUSE_BUTTON_LAST)
            return;

        if (!m_is_lock_mods)
            mods &= ~(KeyMod::KEY_MOD_NUM_LOCK | KeyMod::KEY_MOD_CAPS_LOCK);


        if (m_callbacks.mousebutton)
        {
            m_callbacks.mousebutton(button, action, mods);
        }
    }

    void Window::inputCursorPosCallback(double x, double y)
    {
        if (m_callbacks.cursorPos)
        {
            m_callbacks.cursorPos(x, y);
        }
    }

    void Window::inputCursorEnterCallback(bool entered)
    {
        if (m_callbacks.cursorEnter)
        {
            m_callbacks.cursorEnter(entered);
        }
    }

}