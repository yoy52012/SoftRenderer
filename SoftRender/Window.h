#pragma once

#include <windows.h>
#include <direct.h>

#include <string>

#include "FrameBuffer.h"

#define UNUSED_VAR(x) ((void)(x))



namespace SoftRenderer
{
	class Window
	{
	public:
		static Window* create(const std::string& title, int width, int height);

		Window(const std::string& title, int width, int height);

		~Window();

		void destroy();

		bool should_close();

		void drawBuffer(FrameBuffer& buffer);



	private:
		void init(const std::string& title, int width, int height);

		HWND createWindow(const std::string& title, int width, int height);

		void createSurface(HWND handle, int width, int height, HDC* out_memory_dc);

		void presentSurface();


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
	};
}
