#include "stdafx.h"
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <dinput.h>

static bool ready;
static bool verbose;
static HMODULE DirectInput8_base;

typedef HRESULT WINAPI DirectInput8Create_t(HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID* ppvOut,
    LPUNKNOWN punkOuter
);

static wchar_t* title = L"DirectInput8 Hook";
static DirectInput8Create_t* DirectInput8Create_base;

static void showMessage(wchar_t* msg, UINT uType = MB_ICONERROR) {
    MessageBoxW(NULL, msg, title, MB_OK | uType);
}

static void showFolderPathError(HRESULT hr) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to get system folder path using SHGetFolderPathA!"
        L"\nHRESULT is %d (check MSDN for error codes).", hr);
    showMessage(buf);
}

static void showLibraryLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to load the real dinput8.dll using LoadLibraryW!"
        L"\nPath is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

static void showFunctionLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to locate the real DirectInput8Create function!"
        L"\nDLL path is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

template<typename Tc>
void fakestrncat(Tc* dst, const Tc* src, size_t dstSize) {
    size_t dstLen = 0;
    while (dst[dstLen]) dstLen++;
    if (dstLen >= dstSize - 1) return;

    size_t srcLen = 0;
    while (src[srcLen]) srcLen++;
    if (srcLen == 0) return;

    size_t toCopy = srcLen;
    if (dstLen + toCopy + 1 > dstSize) {
        toCopy = dstSize - dstLen - 1;
    }
    memcpy_arr(dst + dstLen, src, toCopy);
    dst[dstLen + toCopy] = 0;
}

template<typename Tc, size_t size>
inline void fakestrncat(Tc(&dst)[size], const Tc* src) {
    fakestrncat(dst, src, size);
}

static HMODULE loadBaseDLL() {
    ready = true;
    wchar_t path[MAX_PATH];
    int csidl;
#ifndef _WIN64
    csidl = CSIDL_SYSTEMX86;
#else
    csidl = CSIDL_SYSTEM;
#endif
    auto hr = SHGetFolderPathW(NULL, csidl, NULL, 0, path);
    if (hr != S_OK) {
        showFolderPathError(hr);
        return NULL;
    }
    fakestrncat(path, L"\\dinput8.dll");
    auto hm = LoadLibraryW(path);
    if (hm == NULL) {
        showLibraryLoadError(path, GetLastError());
        return NULL;
    }
    DirectInput8_base = hm;
    auto f = (DirectInput8Create_t*)GetProcAddress(hm, "DirectInput8Create");
    if (f == nullptr) {
        showFunctionLoadError(path, GetLastError());
        return NULL;
    }
    DirectInput8Create_base = f;
    return hm;
}

extern "C" HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter)
{
    if (!ready) {
        if (verbose) {
            showMessage(L"DirectInput8Create is being called for the first time, loading the real dinput8.dll",
                MB_ICONINFORMATION);
        }
        loadBaseDLL();
    }
    if (DirectInput8Create_base != nullptr) {
        return DirectInput8Create_base(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
    else {
        return E_FAIL;
    }
}

static void init() {
    ready = false;
    DirectInput8_base = NULL;
    DirectInput8Create_base = NULL;
    verbose = false;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        init();
        break;
    case DLL_PROCESS_DETACH:
        if (DirectInput8_base)
            FreeLibrary(DirectInput8_base);
        break;
    }
    return TRUE;
}