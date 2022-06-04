
#include "pch.h"
#include <exception>
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>

static HANDLE processHandle = nullptr;
static DWORD pointsAddress = NULL;

DWORD GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID); // make snapshot of all modules within process
    MODULEENTRY32 ModuleEntry32 = { 0 };
    ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &ModuleEntry32)) //store first Module in ModuleEntry32
    {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if Found Module matches Module we look for -> done!
            {
                dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in Snapshot and store in ModuleEntry32


    }
    CloseHandle(hSnapshot);
    return dwModuleBaseAddress;
}

void Initialize()
{
    HWND hGameWindow = FindWindow(NULL, L"Age of Empires Expansion");
    if (hGameWindow == NULL) 
        throw new std::exception("Start the game first!");
    
    DWORD pID = NULL; // ID of our Game
    GetWindowThreadProcessId(hGameWindow, &pID);
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL)  // error handling
        throw new std::exception("Failed to open process");
    

    TCHAR gameName[13];
    wcscpy_s(gameName, 13, L"Empiresx.exe");

    DWORD gameBaseAddress = GetModuleBaseAddress(gameName, pID);
    DWORD offsetGameToBaseAdress = 0x00188144;
    DWORD pointsOffsets[] = { 0x54, 0x0, 0x540, 0x10C, 0x50, 0x0 };
    DWORD baseAddress = NULL;

    ReadProcessMemory(processHandle, (LPVOID)(gameBaseAddress + offsetGameToBaseAdress), &baseAddress, sizeof(baseAddress), NULL);

    pointsAddress = baseAddress; //the Adress we need -> change now while going through offsets
    for (int i = 0; i < 5; i++) // -1 because we dont want the value at the last offset
        ReadProcessMemory(processHandle, (LPVOID)(pointsAddress + pointsOffsets[i]), &pointsAddress, sizeof(pointsAddress), NULL);

    pointsAddress += pointsOffsets[5]; //Add Last offset -> done!!
}

void AddPoint()
{
    float currentPoint = 0;
    ReadProcessMemory(processHandle, (LPVOID)(pointsAddress), &currentPoint, sizeof(currentPoint), NULL);
    float newPoints = currentPoint + 100;
    WriteProcessMemory(processHandle, (LPVOID)(pointsAddress), &newPoints, 4, 0);
}