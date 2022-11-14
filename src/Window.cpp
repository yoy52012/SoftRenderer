#include "Window.h"

#include <iostream>

#include <cassert>
#include <cstring>

#include <windowsx.h>

#include "Input.h"

#define _CRT_SECURE_NO_WARNINGS

#define UNUSED_VAR(x) ((void)(x))


namespace SoftRenderer
{

    struct WindowImpl
    {
        void init(const std::string& title, int width, int height);
        void destroy();

        HWND createNativeWindow(const std::string& title, int width, int height);
        void createSurface(HWND handle, int width, int height, HDC* out_memory_dc);
        void presentSurface();

        void pollEvent();

        void drawBuffer(const FrameBuffer::Ptr& buffer);

        void setWindowTitle(const std::string& title);

        void getWindowSize(int* width, int* height);
        void setWindowSize(int width, int height);

        int getKey(int key);
        int getMouseButton(int button);
        void getCursorPos(double* xpos, double* ypos);
        void setCursorPos(double xpos, double ypos);

        void setKeyCallback(KeyCallback keyCallback);
        void setMouseButtonCallback(MouseButtonCallback mouseButtonCallback);
        void setCursorPosCallback(CursorPositionCallback cursorPositionCallback);
        void setCursorEnterCallback(CursorEnterCallback cursorEnterCallback);
        void setScrollCallback(MouseScrollCallback mouseScrollCallback);


        // Internal Event API
        void inputKeyNotify(int key, int scancode, int action, int mods);
        void inputScrollNotify(double xoffset, double yoffset);
        void inputMouseClickNotify(int button, int action, int mods);
        void inputCursorPosCallback(double x, double y);
        void inputCursorEnterCallback(bool entered);


        void initKeyMaps();

        struct
        {
            KeyCallback key;
            MouseButtonCallback mousebutton;
            MouseScrollCallback scroll;
            CursorPositionCallback cursorPos;
            CursorEnterCallback cursorEnter;
        }mCallbacks;

        int mWidth = 0;

        int mHeight = 0;

        std::array<InputAction, InputKeyCode::KEY_LAST + 1> mKeys;

        std::array<InputAction, InputMouseButton::MOUSE_BUTTON_LAST + 1> mMousebuttons;

        std::unordered_map<int, InputKeyCode> mKeycodeMap;

        bool mIsLockMods = false;

        unsigned char* mFrameData = nullptr;

        HWND mHandle;
        HDC  mMemoryDc;

        bool mIsCursorTracked;

        // The last received cursor position, regardless of source
        int mLastCursorPosX;
        int mLastCursorPosY;
    };

    Window* Window::create(const std::string& title, int width, int height)
    {
        Window* window = new Window(title, width, height);
        window->init(title, width, height);
        return window;
    }

    Window::Window()
    {
    }

    Window::Window(const std::string& title, int width, int height)
    {
        mWidth = width;
        mHeight = height;
    }

    Window::~Window()
    {
    }

    void Window::init(const std::string& title, int width, int height)
    {
        mImpl->init(title, width, height);
    }

    void Window::destroy()
    {
        mImpl->destroy();
    }

    bool Window::shouldClose()
    {
        return mShouldClose;
    }

    void Window::drawBuffer(const FrameBuffer::Ptr& buffer)
    {
        mImpl->drawBuffer(buffer);
    }

    void Window::pollEvent()
    {
        mImpl->pollEvent();
    }

    void Window::setWindowTitle(const std::string& title)
    {
        mImpl->setWindowTitle(title);
    }

    void Window::getWindowSize(int* width, int* height)
    {
        mImpl->getWindowSize(width, height);
    }

    void Window::setWindowSize(int width, int height)
    {
        mImpl->setWindowSize(width, height);
    }

    int Window::getKeyAction(int key)
    {
        return mImpl->getKey(key);
    }

    int Window::getMouseButtonAction(int button)
    {
        return mImpl->getMouseButton(button);
    }

    void Window::getCursorPos(double* xpos, double* ypos)
    {
        return mImpl->getCursorPos(xpos, ypos);
    }

    void Window::setCursorPos(double xpos, double ypos)
    {
        return mImpl->setCursorPos(xpos, ypos);
    }

    void Window::bindKeyCallback()
    {
        auto keyCallback = [this](int key, int scancode, int action, int mods)
        {
            if (action == InputAction::PRESS)
                this->mKeyPressedEvent.invoke(key);

            if (action == InputAction::RELEASE)
                this->mKeyReleasedEvent.invoke(key);
        };

        mImpl->setKeyCallback(keyCallback);
    }

    void Window::bindMouseButtonCallbcak()
    {
        auto mouseButtonCallback = [this](int button, int action, int mods)
        {

            if (action == InputAction::PRESS)
                this->mMouseButtonPressedEvent.invoke(button);

            if (action == InputAction::RELEASE)
                this->mMouseButtonReleasedEvent.invoke(button);
        };

        mImpl->setMouseButtonCallback(mouseButtonCallback);
    }

    void Window::bindCursorMoveCallback()
    {
        auto cursorMoveCallback = [this](double x, double y)
        {
            this->mCursorMoveEvent.invoke(static_cast<float>(x), static_cast<float>(y));
        };

        mImpl->setCursorPosCallback(cursorMoveCallback);
    }

    

    /************************************************************************/
    /*                Platform Code                                         */
    /************************************************************************/

#ifdef UNICODE
    static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
    static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
#else
    static const char* const WINDOW_CLASS_NAME = "Class";
    static const char* const WINDOW_ENTRY_NAME = "Entry";
#endif

#define LINE_SIZE 256


    // Returns the window style for the specified window
    static DWORD getWindowStyle(const WindowImpl* window)
    {
        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        style |= WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_MAXIMIZEBOX | WS_THICKFRAME;
        return style;
    }

    // Returns the extended window style for the specified window
    static DWORD getWindowExStyle(const WindowImpl* window)
    {
        DWORD style = WS_EX_APPWINDOW;
        return style;
    }

    static int getKeyMods(void)
    {
        int mods = 0;

        if (GetKeyState(VK_SHIFT) & 0x8000)
            mods |= InputKeyMod::KEY_MOD_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000)
            mods |= InputKeyMod::KEY_MOD_CONTROL;
        if (GetKeyState(VK_MENU) & 0x8000)
            mods |= InputKeyMod::KEY_MOD_ALT;
        if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
            mods |= InputKeyMod::KEY_MOD_SUPER;
        if (GetKeyState(VK_CAPITAL) & 1)
            mods |= InputKeyMod::KEY_MOD_CAPS_LOCK;
        if (GetKeyState(VK_NUMLOCK) & 1)
            mods |= InputKeyMod::KEY_MOD_NUM_LOCK;

        return mods;
    }

    void WindowImpl::init(const std::string& title, int width, int height)
    {
        initKeyMaps();

        HWND handle;
        HDC memory_dc;

        handle = createNativeWindow(title, width, height);
        createSurface(handle, width, height, &memory_dc);

        mHandle = handle;
        mMemoryDc = memory_dc;
        mWidth = width;
        mHeight = height;

        SetProp(handle, WINDOW_ENTRY_NAME, this);

        ShowWindow(handle, SW_SHOW);
    }

    void WindowImpl::destroy()
    {
        ShowWindow(mHandle, SW_HIDE);
        RemoveProp(mHandle, WINDOW_ENTRY_NAME);

        DeleteDC(mMemoryDc);
        DestroyWindow(mHandle);

        mFrameData = nullptr;
        //free(m_frame_data);
    }

    void WindowImpl::pollEvent()
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

    void WindowImpl::drawBuffer(const FrameBuffer::Ptr& buffer)
    {
        auto blitBufferData = [](const FrameBuffer::Ptr& src, unsigned char* dst) {
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

        blitBufferData(buffer, mFrameData);

        presentSurface();
    }

    void WindowImpl::setWindowTitle(const std::string& title)
    {
        auto createWideStringFromUTF8Win32 = [](const char* source)->WCHAR*
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
        };


        WCHAR* wideTitle = createWideStringFromUTF8Win32(title.c_str());
        if (!wideTitle)
            return;

        SetWindowTextW(mHandle, wideTitle);
        free(wideTitle);
    }

    void WindowImpl::getWindowSize(int* width, int* height)
    {
        RECT area;
        GetClientRect(mHandle, &area);

        if (width)
            *width = area.right;
        if (height)
            *height = area.bottom;
    }

    void WindowImpl::setWindowSize(int width, int height)
    {
        assert(width >= 0);
        assert(height >= 0);

        RECT rect = { 0, 0, width, height };
        AdjustWindowRectExForDpi(&rect, getWindowStyle(this), FALSE, getWindowExStyle(this), GetDpiForWindow(mHandle));

        SetWindowPos(mHandle, HWND_TOP,
            0, 0, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }

    int WindowImpl::getKey(int key)
    {
        if (key < InputKeyCode::KEY_SPACE || key > InputKeyCode::KEY_LAST)
        {
            std::cerr << "Invalid key " << key << std::endl;
            return InputAction::RELEASE;
        }
        return (int)mKeys[key];
    }

    int WindowImpl::getMouseButton(int button)
    {
        if (button < -1 || button > InputMouseButton::MOUSE_BUTTON_LAST)
        {
            std::cerr << "Invalid mouse button " << button << std::endl;
            return InputAction::RELEASE;
        }

        return (int)mMousebuttons[button];
    }

    void WindowImpl::getCursorPos(double* xpos, double* ypos)
    {
        if (xpos)
            *xpos = 0;
        if (ypos)
            *ypos = 0;

        POINT pos;

        if (GetCursorPos(&pos))
        {
            ScreenToClient(mHandle, &pos);

            if (xpos)
                *xpos = pos.x;
            if (ypos)
                *ypos = pos.y;
        }
    }

    void WindowImpl::setCursorPos(double xpos, double ypos)
    {
        if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX ||
            ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
        {
           std::cerr <<  "Invalid cursor position " << "(" << xpos << "," << ypos << ")" << std::endl;
            return;
        }

        //if (!platform.windowFocused(window))
        //    return;

        POINT pos = { (int)xpos, (int)ypos };

        ClientToScreen(mHandle, &pos);
        SetCursorPos(pos.x, pos.y);

        mLastCursorPosX = pos.x;
        mLastCursorPosY = pos.y;
    }

    void WindowImpl::initKeyMaps()
    {
        mKeycodeMap =
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



    LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WindowImpl* window = (WindowImpl*)GetProp(hWnd, WINDOW_ENTRY_NAME);

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

            // HACK: Alt+PrtSc has a different scancode than just PrtSc
            if (scancode == 0x54)
                scancode = 0x137;

            // HACK: Ctrl+Pause has a different scancode than just Pause
            if (scancode == 0x146)
                scancode = 0x45;

            // HACK: CJK IME sets the extended bit for right Shift
            if (scancode == 0x136)
                scancode = 0x36;

            key = window->mKeycodeMap[scancode];

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
                    // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                    // HACK: We only want one event for Alt Gr, so if we detect
                    //       this sequence we discard this Left Ctrl message now
                    //       and later report Right Alt normally
                    MSG next;
                    const DWORD time = GetMessageTime();

                    if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                    {
                        if (next.message == WM_KEYDOWN ||
                            next.message == WM_SYSKEYDOWN ||
                            next.message == WM_KEYUP ||
                            next.message == WM_SYSKEYUP)
                        {
                            if (next.wParam == VK_MENU &&
                                (HIWORD(next.lParam) & KF_EXTENDED) &&
                                next.time == time)
                            {
                                // Next message is Right Alt down so discard this
                                break;
                            }
                        }
                    }

                    key = InputKeyCode::KEY_LEFT_CONTROL;
                }
            }
            else if (wParam == VK_PROCESSKEY)
            {
                // IME notifies that keys have been filtered by setting the
                // virtual key-code to VK_PROCESSKEY
                break;
            }

            if (action == InputAction::RELEASE && wParam == VK_SHIFT)
            {
                // HACK: Release both Shift keys on Shift up event, as when both
                //       are pressed the first release does not emit any event
                // NOTE: The other half of this is in _glfwPollEventsWin32
                window->inputKeyNotify(InputKeyCode::KEY_LEFT_SHIFT, scancode, action, mods);
                window->inputKeyNotify(InputKeyCode::KEY_RIGHT_SHIFT, scancode, action, mods);
            }
            else if (wParam == VK_SNAPSHOT)
            {
                // HACK: Key down is not reported for the Print Screen key
                window->inputKeyNotify(key, scancode, InputAction::PRESS, mods);
                window->inputKeyNotify(key, scancode, InputAction::RELEASE, mods);
            }
            else
            {
                window->inputKeyNotify(key, scancode, action, mods);
            }

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
                if (window->mMousebuttons[i] == InputAction::PRESS)
                    break;
            }

            if (i > InputMouseButton::MOUSE_BUTTON_LAST)
            {
                SetCapture(hWnd);
            }

            window->inputMouseClickNotify(button, action, getKeyMods());

            for (i = 0; i <= InputMouseButton::MOUSE_BUTTON_LAST; i++)
            {
                if (window->mMousebuttons[i] == InputAction::PRESS)
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
                tme.hwndTrack = window->mHandle;
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

    HWND WindowImpl::createNativeWindow(const std::string& title, int width, int height)
    {
        // https://blog.csdn.net/bizhu12/article/details/6667580       
        // Registe Class 
        WNDCLASS window_class;
        ZeroMemory(&window_class, sizeof(window_class));
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = WndProc;
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

    void WindowImpl::createSurface(HWND handle, int width, int height, HDC* out_memory_dc)
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
            DIB_RGB_COLORS, (void**)&mFrameData,
            nullptr, 0);
        assert(dib_bitmap != nullptr);
        old_bitmap = (HBITMAP)SelectObject(memory_dc, dib_bitmap);
        DeleteObject(old_bitmap);

        *out_memory_dc = memory_dc;
    }

    void WindowImpl::presentSurface()
    {
        HDC windowDc = GetDC(mHandle);
        HDC memoryDc = mMemoryDc;
        //auto surface = window->surface;
        int width = mWidth;
        int height = mHeight;
        BitBlt(windowDc, 0, 0, width, height, memoryDc, 0, 0, SRCCOPY);
        ReleaseDC(mHandle, windowDc);
    }

    void WindowImpl::setKeyCallback(KeyCallback keyCallback)
    {
        mCallbacks.key = keyCallback;
    }

    void WindowImpl::setMouseButtonCallback(MouseButtonCallback mouseButtonCallback)
    {
        mCallbacks.mousebutton = mouseButtonCallback;
    }

    void WindowImpl::setCursorPosCallback(CursorPositionCallback cursorPositionCallback)
    {
        mCallbacks.cursorPos = cursorPositionCallback;
    }

    void WindowImpl::setCursorEnterCallback(CursorEnterCallback cursorEnterCallback)
    {
        mCallbacks.cursorEnter = cursorEnterCallback;
    }

    void WindowImpl::setScrollCallback(MouseScrollCallback mouseScrollCallback)
    {
        mCallbacks.scroll = mouseScrollCallback;
    }

    void WindowImpl::inputKeyNotify(int key, int scancode, int action, int mods)
    {
        assert(key >= 0 || key == KEY_UNKNOWN);
        assert(key <= KEY_LAST);
        assert(action == InputAction::PRESS || action == InputAction::RELEASE);

        if (key >= 0 && key <= InputKeyCode::KEY_LAST)
        {
            bool repeated = false;

            if (action == InputAction::RELEASE && mKeys[key] == InputAction::RELEASE)
            {
                return;
            }

            if (action == InputAction::PRESS && mKeys[key] == InputAction::PRESS)
            {
                repeated = true;
            }

            mKeys[key] = (InputAction)action;

            if (repeated)
            {
                action = InputAction::REPEAT;
            }
        }

        if (!mIsLockMods)
        {
            mods &= ~(InputKeyMod::KEY_MOD_NUM_LOCK | InputKeyMod::KEY_MOD_CAPS_LOCK);
        }

        if (mCallbacks.key)
        {
            mCallbacks.key(key, scancode, action, mods);
        }
    }

    void WindowImpl::inputScrollNotify(double xoffset, double yoffset)
    {
        assert(xoffset > -std::numeric_limits<double>::max());
        assert(xoffset < std::numeric_limits<double>::max());
        assert(yoffset > -std::numeric_limits<double>::max());
        assert(yoffset < std::numeric_limits<double>::max());

        if (mCallbacks.scroll)
            mCallbacks.scroll(xoffset, yoffset);
    }

    void WindowImpl::inputMouseClickNotify(int button, int action, int mods)
    {
        assert(button >= 0);
        assert(button <= InputMouseButton::MOUSE_BUTTON_LAST);
        assert(action == InputAction::PRESS || action == InputAction::RELEASE);

        if (button < 0 || button > InputMouseButton::MOUSE_BUTTON_LAST)
            return;

        if (!mIsLockMods)
            mods &= ~(InputKeyMod::KEY_MOD_NUM_LOCK | InputKeyMod::KEY_MOD_CAPS_LOCK);

        mMousebuttons[button] = (InputAction)action;

        if (mCallbacks.mousebutton)
        {
            mCallbacks.mousebutton(button, action, mods);
        }
    }

    void WindowImpl::inputCursorPosCallback(double x, double y)
    {
        assert(x > -std::numeric_limits<double>::max());
        assert(x < std::numeric_limits<double>::max());
        assert(y > -std::numeric_limits<double>::max());
        assert(y < std::numeric_limits<double>::max());

        if (mCallbacks.cursorPos)
        {
            mCallbacks.cursorPos(x, y);
        }
    }

    void WindowImpl::inputCursorEnterCallback(bool entered)
    {
        if (mCallbacks.cursorEnter)
        {
            mCallbacks.cursorEnter(entered);
        }
    }

}