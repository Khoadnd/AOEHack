// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "GameHacking.h"

//static bool stop = false;

DWORD WINAPI main_thread(LPVOID param)
{
    Initialize();
    while(true)
    {
	    if (GetAsyncKeyState(VK_F6) & 0x80000)
	    {
            AddPoint();
        }
        Sleep(100);
	    
    }
    //return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(0, "DLL Injected", "DLL Injected", 0);
        CreateThread(0, 0, main_thread, hModule, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        //stop = true;
        break;
    default: ;
    }
    return TRUE;
}

