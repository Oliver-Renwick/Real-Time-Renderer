#pragma once

#include <windows.h>
#include<iostream>
#include <cstring>
#include<string>
#include "Engine.h"


#ifdef WIN32


#define WIN_MAIN()

static bool Running;

tde::Engine* m_engine = nullptr;

LRESULT CALLBACK mainWindowCallBack(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_engine != nullptr)
    {
        m_engine->Handle_Message(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);    

}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    m_engine = new tde::Engine(hInst, mainWindowCallBack);
    m_engine->Init();
    std::cout << PROJECT_PATH << std::endl;
    m_engine->Run();


    delete m_engine;
    std::cin.get();
    return 0;
}

#endif // WIN32