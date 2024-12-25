#include "stdafx.h"
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <dinput.h>

// ȫ�ֱ���
static bool ready; // ָʾ�Ƿ��Ѽ�����ʵ��dinput8.dll
static bool verbose; // ָʾ�Ƿ���ʾ��ϸ��Ϣ
static HMODULE DirectInput8_base; // ��ʵ��dinput8.dllģ����

// DirectInput8Create����ָ������
typedef HRESULT WINAPI DirectInput8Create_t(HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID* ppvOut,
    LPUNKNOWN punkOuter
);

// ȫ�ֱ���
static const wchar_t* title = L"DirectInput8 Hook"; // ��Ϣ�����
static DirectInput8Create_t* DirectInput8Create_base; // ��ʵ��DirectInput8Create����ָ��

// ��ʾ��Ϣ��
static void showMessage(const wchar_t* msg, UINT uType = MB_ICONERROR) {
    MessageBoxW(NULL, msg, title, MB_OK | uType);
}

// ��ʾ��ȡϵͳ�ļ���·��ʧ�ܵ���Ϣ
static void showFolderPathError(HRESULT hr) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to get system folder path using SHGetFolderPathA!"
        L"\nHRESULT is %d (check MSDN for error codes).", hr);
    showMessage(buf);
}

// ��ʾ����dinput8.dllʧ�ܵ���Ϣ
static void showLibraryLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to load the real dinput8.dll using LoadLibraryW!"
        L"\nPath is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

// ��ʾ��ȡDirectInput8Create������ַʧ�ܵ���Ϣ
static void showFunctionLoadError(wchar_t* path, DWORD gle) {
    wchar_t buf[1024];
    wsprintfW(buf, L"Failed to locate the real DirectInput8Create function!"
        L"\nDLL path is \"%s\""
        L"\nGetLastError is %d (check MSDN for error codes).", path, gle);
    showMessage(buf);
}

// �ַ���ƴ�Ӻ���ģ�壬���ڱ��⻺�������
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
    memcpy_arr(dst + dstLen, src, toCopy); // ����memcpy_arr��һ����ȫ���ڴ渴�ƺ�������ֹ���������
    dst[dstLen + toCopy] = 0;
}

// �ַ���ƴ�Ӻ���ģ�壬���ڹ̶���С������
template<typename Tc, size_t size>
inline void fakestrncat(Tc(&dst)[size], const Tc* src) {
    fakestrncat(dst, src, size);
}

// ������ʵ��dinput8.dll
static HMODULE loadBaseDLL() {
    ready = true; // ����Ѽ���
    wchar_t path[MAX_PATH];
    int csidl;
#ifndef _WIN64 // �ж��Ƿ�Ϊ64λϵͳ
    csidl = CSIDL_SYSTEMX86; // 32λϵͳ
#else
    csidl = CSIDL_SYSTEM; // 64λϵͳ
#endif
    auto hr = SHGetFolderPathW(NULL, csidl, NULL, 0, path); // ��ȡϵͳ�ļ���·��
    if (hr != S_OK) {
        showFolderPathError(hr); // ��ʾ������Ϣ
        return NULL;
    }
    fakestrncat(path, L"\\dinput8.dll"); // ƴ��dinput8.dll·��
    auto hm = LoadLibraryW(path); // ����dinput8.dll
    if (hm == NULL) {
        showLibraryLoadError(path, GetLastError()); // ��ʾ������Ϣ
        return NULL;
    }
    DirectInput8_base = hm; // ����ģ����
    auto f = (DirectInput8Create_t*)GetProcAddress(hm, "DirectInput8Create"); // ��ȡDirectInput8Create������ַ
    if (f == nullptr) {
        showFunctionLoadError(path, GetLastError()); // ��ʾ������Ϣ
        return NULL;
    }
    DirectInput8Create_base = f; // ���溯��ָ��
    return hm;
}

// ��ȡDirectInput8Create����
extern "C" HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter)
{
    if (!ready) { // �����δ������ʵ��dinput8.dll
        if (verbose) { // �����������ϸ��Ϣ
            showMessage(L"DirectInput8Create is being called for the first time, loading the real dinput8.dll",
                MB_ICONINFORMATION); // ��ʾ��Ϣ
        }
        loadBaseDLL(); // ������ʵ��dinput8.dll
    }
    if (DirectInput8Create_base != nullptr) { // ����Ѽ���
        return DirectInput8Create_base(hinst, dwVersion, riidltf, ppvOut, punkOuter); // ������ʵ��DirectInput8Create����
    }
    else {
        return E_FAIL; // ����ʧ��
    }
}

// ��ʼ��ȫ�ֱ���
static void init() {
    ready = false;
    DirectInput8_base = NULL;
    DirectInput8Create_base = NULL;
    verbose = false;
}

// DLL��ڵ㺯��
BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH: // DLL����ʱ
        DisableThreadLibraryCalls(hModule); // �����߳�֪ͨ
        init(); // ��ʼ��ȫ�ֱ���
        break;
    case DLL_PROCESS_DETACH: // DLLж��ʱ
        if (DirectInput8_base)
            FreeLibrary(DirectInput8_base); // �ͷ�dinput8.dll
        break;
    }
    return TRUE;
}