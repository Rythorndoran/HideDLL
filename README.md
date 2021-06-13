# HideDLL
Hide dynamic link library , support x64 and x86

How to use

Unlinking a loaded DLL from PEB:

UnlinkModule((void*)hModule);

Erases headers from memory:

RemovePeHeader(hModule);
