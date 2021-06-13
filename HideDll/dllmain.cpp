// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include <iostream>
char buff[512];
//===========================================================================
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
//===========================================================================
typedef struct _PEB_LDR_DATA
{
    ULONG           Length;
    BOOLEAN         Initialized;
    PVOID           SsHandle;
    LIST_ENTRY      InLoadOrderModuleList;
    LIST_ENTRY      InMemoryOrderModuleList;
    LIST_ENTRY      InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;
//===========================================================================
typedef struct _LDR_MODULE
{
    LIST_ENTRY      InLoadOrderModuleList;
    LIST_ENTRY      InMemoryOrderModuleList;
    LIST_ENTRY      InInitializationOrderModuleList;
    PVOID           BaseAddress;
    PVOID           EntryPoint;
    ULONG           SizeOfImage;
    UNICODE_STRING  FullDllName;
    UNICODE_STRING  BaseDllName;
    ULONG           Flags;
    SHORT           LoadCount;
    SHORT           TlsIndex;
    LIST_ENTRY      HashTableEntry;
    ULONG           TimeDateStamp;
} LDR_MODULE, * PLDR_MODULE;
//===========================================================================

void UnlinkModule(void* Module)
{
    DWORD dwPEB = 0, dwOffset = 0;
    PLIST_ENTRY pUserModuleHead = NULL, pUserModule = NULL;
    PPEB_LDR_DATA pLdrData;
    PLDR_MODULE pLdrModule = NULL;
    void* ModuleBase = NULL;
    int i = 0, n = 0;
#ifndef _WIN64
    _asm
    {
        pushad
        mov eax, fs: [48]
        mov dwPEB, eax
        popad
    }

    pLdrData = (PPEB_LDR_DATA)(PDWORD)(*(PDWORD)(dwPEB + 12));
#else
    BYTE* _teb = (BYTE*)__readgsqword(0x30);
    pLdrData = (PPEB_LDR_DATA)(PULONGLONG)(*(PULONGLONG)((*(PULONGLONG)(_teb + 0x60)) + 0x18));
#endif  

    for (; i < 3; i++)
    {
        switch (i)
        {
        case 0:
            pUserModuleHead = (PLIST_ENTRY)(&(pLdrData->InLoadOrderModuleList));
            pUserModule = (PLIST_ENTRY)(&(pLdrData->InLoadOrderModuleList));
            dwOffset = 0;
            break;

        case 1:
            pUserModuleHead = (PLIST_ENTRY)(&(pLdrData->InMemoryOrderModuleList));
            pUserModule = (PLIST_ENTRY)(&(pLdrData->InMemoryOrderModuleList));
#ifndef _WIN64
            dwOffset = 8;
#else
            dwOffset = 16;
#endif
            break;
        case 2:
            pUserModuleHead = pUserModule = (PLIST_ENTRY)(&(pLdrData->InInitializationOrderModuleList));
#ifndef _WIN64
            dwOffset = 16;
#else
            dwOffset = 32;
#endif
            break;
        }
        while (pUserModule->Flink != pUserModuleHead)
        {
            pUserModule = pUserModule->Flink;
#ifndef _WIN64
            ModuleBase = *(void**)(((DWORD)(pUserModule)) + (24 - dwOffset));
#else
            ModuleBase = *(void**)(((LONGLONG)(pUserModule)) + (48 - dwOffset));
#endif          
            if (ModuleBase == Module)
            {
#ifndef _WIN64
                if (!pLdrModule)
                    pLdrModule = (PLDR_MODULE)(((DWORD)(pUserModule)) - dwOffset);
#else
                if (!pLdrModule)
                    pLdrModule = (PLDR_MODULE)(((LONGLONG)(pUserModule)) - dwOffset);
#endif              
                pUserModule->Blink->Flink = pUserModule->Flink;
                pUserModule->Flink->Blink = pUserModule->Blink;
            }
        }
    }
    // Unlink from LdrpHashTable
    if (pLdrModule)
    {
        pLdrModule->HashTableEntry.Blink->Flink = pLdrModule->HashTableEntry.Flink;
        pLdrModule->HashTableEntry.Flink->Blink = pLdrModule->HashTableEntry.Blink;
    }
}
//===========================================================================
void RemovePeHeader(HANDLE GetModuleBase)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleBase;
    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

    if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
        return;

    if (pNTHeader->FileHeader.SizeOfOptionalHeader)
    {
        DWORD Protect;
        WORD Size = pNTHeader->FileHeader.SizeOfOptionalHeader;
        VirtualProtect((void*)GetModuleBase, Size, PAGE_EXECUTE_READWRITE, &Protect);
        RtlZeroMemory((void*)GetModuleBase, Size);
        VirtualProtect((void*)GetModuleBase, Size, Protect, &Protect);
    }
}
//===========================================================================



void Thread()
{
    while (true)
    {
        MessageBoxA(NULL, buff, "10s一次弹窗证明我还在运行！", MB_OK);
        Sleep(10000);
    }
}





BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {

        DisableThreadLibraryCalls(hModule);
        RemovePeHeader(hModule);
        UnlinkModule((void*)hModule);
        sprintf_s(buff,"DLL地址是：%p", hModule);
        MessageBox(NULL, L"Inject!", L"", MB_OK);
        CloseHandle(CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Thread), nullptr, 0, nullptr));
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
//===========================================================================

