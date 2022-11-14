#pragma once

#include <windows.h>
#include <direct.h>

#include <string>
#include <array>
#include <functional>
#include <unordered_map>

#include "FrameBuffer.h"
#include "Input.h"

#include "Event.h"



namespace SoftRenderer
{
	using KeyCallback = std::function<void(int, int, int, int)>;
	using MouseButtonCallback = std::function<void(int, int, int)>;
	using MouseScrollCallback = std::function<void(double, double)>;
	using CursorPositionCallback = std::function<void(double, double)>;
	using CursorEnterCallback = std::function<void(bool)>;

    struct WindowImpl;

	class Window
	{
	public:
		static Window* create(const std::string& title, int width, int height);

		Window();

		Window(const std::string& title, int width, int height);

		~Window();

		void destroy();

		bool shouldClose();

		void drawBuffer(const FrameBuffer::Ptr& buffer);

		void pollEvent();

		void setWindowTitle(const std::string& title);

		void getWindowSize(int* width, int* height);

		void setWindowSize(int width, int height);

		int getKeyAction(int key);

		int getMouseButtonAction(int button);

		void getCursorPos(double* xpos, double* ypos);

		void setCursorPos(double xpos, double ypos);

    private:
		void init(const std::string& title, int width, int height);

        void bindKeyCallback();
        void bindMouseButtonCallbcak();
        void bindCursorMoveCallback();

	private:
		std::unique_ptr<WindowImpl> mImpl = std::make_unique<WindowImpl>();

		int mWidth = 0;
		int mHeight = 0;

		bool mShouldClose = false;

	public:
		Event<int> mKeyPressedEvent;
		Event<int> mKeyReleasedEvent;
		Event<int> mMouseButtonPressedEvent;
		Event<int> mMouseButtonReleasedEvent;
		Event<float, float> mCursorMoveEvent;
	};
}
