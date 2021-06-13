#include<iostream>
#include<windows.h>
#include <Tlhelp32.h>
using namespace std;
int Enumerate_all_loaded_DLLs()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return 0;
	}
	MODULEENTRY32 mi;
	mi.dwSize = sizeof(MODULEENTRY32);
	BOOL bRet = Module32First(hSnapshot, &mi);
	while (bRet)
	{
		std::wcout.imbue(std::locale("chs"));
		cout << hex << "dwSize:" << (DWORD)mi.dwSize << endl;
		cout << hex << "th32ModuleID:" << (DWORD)mi.th32ModuleID << endl;
		cout << hex << "th32ProcessID:" << (DWORD)mi.th32ProcessID << endl;
		cout << hex << "GlblcntUsage:" << (DWORD)mi.GlblcntUsage << endl;
		cout << hex << "ProccntUsage:" << (DWORD)mi.ProccntUsage << endl;
		cout << hex << "modBaseAddr:" << (ULONG32)mi.modBaseAddr << endl;
		cout << hex << "modBaseSize:" << (DWORD)mi.modBaseSize << endl;
		cout << hex << "hModule:" << (HMODULE)mi.hModule << endl;
		wcout << "szModule:" << mi.szModule << endl;
		wcout << "szExePath:" << mi.szExePath << endl;
		cout << "" << endl;
		bRet = Module32Next(hSnapshot, &mi);
	}
}
int main(int argc, char* argv[])
{
	Enumerate_all_loaded_DLLs();
	HMODULE Dll = LoadLibrary(L"HideDll.dll");
	cout << hex << "LoadDllBase:" << (HMODULE)Dll << endl;
	while (true)
	{
		Enumerate_all_loaded_DLLs();
		Sleep(15000);
	}
    system("pause");
    return 0;
}