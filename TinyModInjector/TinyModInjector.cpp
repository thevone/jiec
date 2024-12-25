#include "stdafx.h"
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <dinput.h>

// 全局变量
static bool ready; // 指示是否已加载真实的dinput8.dll
static bool verbose; // 指示是否显示详细消息
static HMODULE DirectInput8_base; // 真实的dinput8.dll模块句柄

// DirectInput8Create函数指针类型
typedef HRESULT WINAPI DirectInput8Create_t(HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID* ppvOut,
    LPUNKNOWN punkOuter
);

// 全局变量
static const wchar_t* title = L"DirectInput8 Hook"; // 消息框标题
static DirectInput8Create_t* DirectInput8Create_base; // 真实的DirectInput8Create函数指针

// 显示消息框
static void showMessage(const wchar_t* msg, UINT uType = MB_ICONERROR) {
    MessageBoxW(NULL, msg, title, MB_OK | uType);
}

// 显示获取系统文件夹路径失败的消息
static void showFolderPathError(HRESULT hr) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to get system folder path using SHGetFolderPathA!"
        L"\nHRESULT is %d (check MSDN for error codes).", hr);
    showMessage(buf);
}

// 显示加载dinput8.dll失败的消息
static void showLibraryLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to load the real dinput8.dll using LoadLibraryW!"
        L"\nPath is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

// 显示获取DirectInput8Create函数地址失败的消息
static void showFunctionLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to locate the real DirectInput8Create function!"
        L"\nDLL path is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

// 字符串拼接函数模板，用于避免缓冲区溢出
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
    memcpy_arr(dst + dstLen, src, toCopy); // 假设memcpy_arr是一个安全的内存复制函数，防止缓冲区溢出
    dst[dstLen + toCopy] = 0;
}

// 字符串拼接函数模板，用于固定大小的数组
template<typename Tc, size_t size>
inline void fakestrncat(Tc(&dst)[size], const Tc* src) {
    fakestrncat(dst, src, size);
}

// 加载真实的dinput8.dll
static HMODULE loadBaseDLL() {
    ready = true; // 标记已加载
    wchar_t path[MAX_PATH];
    int csidl;
#ifndef _WIN64 // 判断是否为64位系统
    csidl = CSIDL_SYSTEMX86; // 32位系统
#else
    csidl = CSIDL_SYSTEM; // 64位系统
#endif
    auto hr = SHGetFolderPathW(NULL, csidl, NULL, 0, path); // 获取系统文件夹路径
    if (hr != S_OK) {
        showFolderPathError(hr); // 显示错误消息
        return NULL;
    }
    fakestrncat(path, L"\\dinput8.dll"); // 拼接dinput8.dll路径
    auto hm = LoadLibraryW(path); // 加载dinput8.dll
    if (hm == NULL) {
        showLibraryLoadError(path, GetLastError()); // 显示错误消息
        return NULL;
    }
    DirectInput8_base = hm; // 保存模块句柄
    auto f = (DirectInput8Create_t*)GetProcAddress(hm, "DirectInput8Create"); // 获取DirectInput8Create函数地址
    if (f == nullptr) {
        showFunctionLoadError(path, GetLastError()); // 显示错误消息
        return NULL;
    }
    DirectInput8Create_base = f; // 保存函数指针
    return hm;
}

// 钩取DirectInput8Create函数
extern "C" HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter)
{
    if (!ready) { // 如果尚未加载真实的dinput8.dll
        if (verbose) { // 如果开启了详细消息
            showMessage(L"DirectInput8Create is being called for the first time, loading the real dinput8.dll",
                MB_ICONINFORMATION); // 显示消息
        }
        loadBaseDLL(); // 加载真实的dinput8.dll
    }
    if (DirectInput8Create_base != nullptr) { // 如果已加载
        return DirectInput8Create_base(hinst, dwVersion, riidltf, ppvOut, punkOuter); // 调用真实的DirectInput8Create函数
    }
    else {
        return E_FAIL; // 返回失败
    }
}

// 初始化全局变量
static void init() {
    ready = false;
    DirectInput8_base = NULL;
    DirectInput8Create_base = NULL;
    verbose = false;
}

// DLL入口点函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH: // DLL加载时
        DisableThreadLibraryCalls(hModule); // 禁用线程通知
        init(); // 初始化全局变量
        break;
    case DLL_PROCESS_DETACH: // DLL卸载时
        if (DirectInput8_base)
            FreeLibrary(DirectInput8_base); // 释放dinput8.dll
        break;
    }
    return TRUE;
}