#pragma once
#include <iostream>
#include<vector>

#include "Renderer/Vulkan_Renderer.h"
#include "Renderer/App/triangle.h"
#include "Renderer/Helper/Inputs/Input_Sytem.h"

namespace tde
{
	class Engine
	{
	public:
		Engine(HINSTANCE hinstance, WNDPROC wndproc);
		~Engine();
		void Init();
		void Run();
		void Handle_Message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		

	private:
		bool isPrepared = true;
		Win* m_windows;
		Renderer* m_renderer;
		triangle* m_triangle;
	private:
		bool m_Running = true;
	};
}