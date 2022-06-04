// Injector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <strsafe.h>
#include <tchar.h>
#include <cassert>

// Get the process ID of desired App
DWORD getProcID(const wchar_t* processName)
{
	// Get Proc ID
	HWND hGameWindow = FindWindow(nullptr, L"Age of Empires Expansion");
	if (hGameWindow == nullptr)
	{
		std::cout << "Start the game!" << std::endl;
		return 0;
	}
	DWORD processID = NULL; // ID of our Game
	GetWindowThreadProcessId(hGameWindow, &processID);
	// End Proc ID

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	// Take snapshots of everything
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	do
	{
		if (wcscmp(pe32.szExeFile, processName) == 0)
		{
			CloseHandle(hSnapShot);
			processID = pe32.th32ParentProcessID;
			break;
		}
	}
	while (Process32Next(hSnapShot, &pe32));
	CloseHandle(hSnapShot);
	return processID;
}

inline bool exist(const std::string& name)
{
	const std::ifstream file(name);
	if (!file) // If the file was not found, then file is 0, i.e. !file=1 or true.
		return false; // The file was not found.
	// If the file was found, then file is non-0.
	return true;
	// The file was found.
}

[[noreturn]]
void errorExit(const wchar_t* lpsz_function)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuffer = new LPVOID;
	const DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&lpMsgBuffer),
		0, nullptr);

	// Display the error message and exit the process

	const auto lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT,
	                                     (lstrlen(static_cast<LPCTSTR>(lpMsgBuffer)) + lstrlen(
		                                     lpsz_function) + 40) * sizeof
	                                     (TCHAR));
	assert(lpDisplayBuf != nullptr);
	StringCchPrintfW(static_cast<LPTSTR>(lpDisplayBuf),
	                 LocalSize(lpDisplayBuf) / sizeof(TCHAR),
	                 TEXT("%s failed with error %d: %s"),
	                 lpsz_function, dw, lpMsgBuffer);
	MessageBox(nullptr, static_cast<LPCTSTR>(lpDisplayBuf), TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuffer);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}


int main()
{
	LPVOID pszLibFileRemote = nullptr;
	const auto process = L"Empiresx.exe";
	const DWORD processID = getProcID(process);

	constexpr char dll[] = "DLL.dll";
	if (!exist(dll))
	{
		std::cout << "debug info: Invalid DLL path!" << std::endl;
	}
	char dllPath[MAX_PATH] = {0}; // full path of DLL
	GetFullPathNameA(dll, MAX_PATH, dllPath, nullptr);
	std::cout << "debug info: Full DLL path: " << dllPath << std::endl;


	const auto processHandler = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, 0, processID);

	if (processHandler)
	{
		pszLibFileRemote = VirtualAllocEx(processHandler, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE,
		                                  PAGE_READWRITE);
	}
	else
	{
		std::cout << "error: Could not open process handle with id:" << processID << std::endl;
	}
	if (pszLibFileRemote == nullptr)
	{
		std::cout << "error: Cannot allocate memory" << std::endl;
	}

	assert(processHandler != nullptr);
	assert(pszLibFileRemote != nullptr);
	const int isWriteOk = WriteProcessMemory(processHandler, pszLibFileRemote, dllPath, strlen(dllPath) + 1,
	                                           nullptr);
	if (!isWriteOk) std::cout << "error: Failed to write" << std::endl;

	const auto threadHandler = CreateRemoteThread(processHandler, nullptr, NULL,
	                                               reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA),
	                                               pszLibFileRemote, NULL, nullptr);

	if (threadHandler == nullptr)
	{
		std::cout << "error: Failed to create thread" << std::endl;
		errorExit(_T("CreateRemoteThread"));
	}

	assert(threadHandler != nullptr);
	WaitForSingleObject(threadHandler, INFINITE);
	CloseHandle(threadHandler);

	VirtualFreeEx(processHandler, dllPath, 0, MEM_RELEASE);
	CloseHandle(processHandler);

	return 0;
}
