#pragma once

#include <windows.h>
#include <direct.h>

#include <string>
#include <array>
#include <functional>
#include <unordered_map>

#include "FrameBuffer.h"
#include "Input.h"

#define UNUSED_VAR(x) ((void)(x))


namespace SoftRenderer
{
	using KeyCallback = std::function<void(int, int, int, int)>;
	using MouseButtonCallback = std::function<void(int, int, int)>;
	using CursorPositionCallback = std::function<void(double, double)>;
	using CursorEnterCallback = std::function<void(bool)>;

	enum KeyMod
	{
		KEY_MOD_SHIFT     = 1,
		KEY_MOD_CONTROL   = 1 << 1,
		KEY_MOD_ALT       = 1 << 2,
		KEY_MOD_SUPER     = 1 << 3,
	    KEY_MOD_CAPS_LOCK = 1 << 4,
		KEY_MOD_NUM_LOCK  = 1 << 5, 
	};

	class Window
	{
	public:
		static Window* create(const std::string& title, int width, int height);

		Window(const std::string& title, int width, int height);

		~Window();

		void destroy();

		bool should_close();

		void drawBuffer(const FrameBuffer::Ptr& buffer);

		void pollEvent();

		void setKeyCallback(KeyCallback keyCallback);
		void setMouseButtonCallback(MouseButtonCallback mouseButtonCallback);
		void setCursorPosCallback(CursorPositionCallback cursorPositionCallback);
		void setCursorEnterCallback(CursorEnterCallback cursorEnterCallback);

		void getCursorPos(double* x, double* y);
		void setCursorPos(double x, double y);

		void setWindowTitle(const std::string& title);

		void getWindowSize(int* width, int* height);
		void setWindowSize(int width, int height);


	private:
		void init(const std::string& title, int width, int height);

		void initKeyMaps();

		HWND createWindow(const std::string& title, int width, int height);

		void createSurface(HWND handle, int width, int height, HDC* out_memory_dc);

		void presentSurface();

	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void inputKeyNotify(int key, int scancode, int action, int mods);
		void inputMouseClickNotify(int button, int action, int mods);
		void inputCursorPosCallback(double x, double y);
		void inputCursorEnterCallback(bool entered);

		
	private:
		struct 
		{
			KeyCallback key;
			MouseButtonCallback mousebutton;
			CursorPositionCallback cursorPos;
			CursorEnterCallback cursorEnter;
		}m_callbacks;


		std::array<InputAction, InputKeyCode::KEY_LAST + 1> m_key_action;

		std::array<InputAction, InputMouseButton::MOUSE_BUTTON_LAST+1> m_mousebutton_action;
		
		std::unordered_map<int, InputKeyCode> m_keycode_map;

		bool m_is_lock_mods;




	private:
		int m_width;
		int m_height;

		unsigned char* m_frame_data;

		HWND m_handle;
		HDC m_memory_dc;

		/* common data */
		bool m_should_close = false;

		//char keys[KEY_NUM];
		//char buttons[BUTTON_NUM];
		//callbacks_t callbacks;
		void* userdata;


		bool mIsCursorTracked;
		// The last received cursor position, regardless of source
		int mLastCursorPosX;
		int mLastCursorPosY;
	};
}
