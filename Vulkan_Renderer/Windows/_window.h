#pragma once
#include <windows.h>
#include <exception>
#include <iostream>

namespace tde
{
	class Win
	{
	public:
		Win(HINSTANCE hInst, WNDPROC wndproc);
		~Win();

		void Setup_Window();

		HWND m_window = nullptr;
		HINSTANCE m_hInst = nullptr;
		WNDPROC m_wndproc = nullptr;
	};
}

