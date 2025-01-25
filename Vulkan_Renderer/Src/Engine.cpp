#include "Engine.h"


namespace tde
{
	Engine::Engine(HINSTANCE hinstance, WNDPROC wndproc)
	{
		OutputDebugStringA("Engine Initialized \n");

		m_windows = new Win(hinstance, wndproc);
        m_renderer = new Renderer(m_windows);
        m_triangle = new triangle();
	}

	Engine::~Engine()
	{
        delete m_triangle;
        delete m_renderer;
        delete m_windows;
		OutputDebugStringA("Engine Terminated \n");
	}


	void Engine::Init()
	{
           //Asset Initialization
        m_triangle->Connect(m_renderer);
        m_triangle->prepare();
	}

	void Engine::Run()
	{
        if (m_windows->m_window)
        {
            MSG msg;
            //Run Loop
            bool quitMessageReceived = false;
            while (!quitMessageReceived) {
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    if (msg.message == WM_QUIT) {
                        quitMessageReceived = true;
                        break;
                    }
                }
     
                 //Input System

                //Game Logic

                //Rendering
                if (!IsIconic(m_windows->m_window) && isPrepared)
                {
                    m_triangle->draw();
                }
                //Frame Info

            }
            vkDeviceWaitIdle(m_renderer->getDevice());
        }
       else
       {
          throw std::runtime_error("Cannot able to run the window handle");
       }
	}

	void Engine::Handle_Message(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
        //Input Source

        switch (uMsg)
        {
        case WM_DESTROY:
        {
            //Handle this as error : recreate Window?
            isPrepared = false;
            m_Running = false;
        }break;
        case WM_CLOSE:
        {
            //Handle this for user to ask questions like "are you sure to close this app"?
            isPrepared = false;
            m_Running = false;
        }break;

        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        }break;

        case WM_MOVE:
        {
            OutputDebugStringA("The Window has been moved\n");
        }break;

        case WM_PAINT:
        {
            ValidateRect(m_windows->m_window, NULL);
        }break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case Key_W:
                keys.forward = true;
                break;
            case Key_S:
                keys.backward = true;
                break;
            case Key_A:
                keys.left = true;
                break;
            case Key_D:
                keys.right = true;
                break;
            case Key_1:
                keys.wireframe = false;
                break;
            case Key_2:
                keys.wireframe = true;
                break;
            case Key_LEFT:
                keys.rot_left = true;
                break;
            case Key_RIGHT:
                keys.rot_right = true;
                break;
            case Key_UP:
                keys.rot_up = true;
                break;
            case Key_DOWN:
                keys.rot_down = true;
                break;
            case Key_Q:
                keys.down = true;
                break;
            case Key_E:
                keys.up = true;
                break;
            }
        }break;

        case WM_MOUSEHWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            OutputDebugStringA("WM_QUIT has been called\n");

            if (delta < 0)
            {
            }
            else if (delta > 0)
            {
                std::cout << "Mouse Scroll Up" << std::endl;
            }
        }break;

        case WM_KEYUP:
        {
            switch (wParam)
            {
            case Key_W:
                keys.forward = false;
                break;
            case Key_S:
                keys.backward = false;
                break;
            case Key_A:
                keys.left = false;
                break;
            case Key_D:
                keys.right = false;
                break;
            case Key_LEFT:
                keys.rot_left = false;
                break;
            case Key_RIGHT:
                keys.rot_right = false;
                break;
            case Key_UP:
                keys.rot_up = false;
                break;
            case Key_DOWN:
                keys.rot_down = false;
                break;
            case Key_Q:
                keys.down = false;
                break;
            case Key_E:
                keys.up = false;
                break;
            }
            break;
        }break;



        }

       // Handle_Message(hwnd, uMsg, wParam, lParam);
	}
}