#pragma once  // ȷ��ͷ�ļ�ֻ������һ��

#ifdef _WINDOWS  // ����� Windows ƽ̨
#include "targetver.h"  // ����Ŀ��ƽ̨�汾��Ϣ

// #define WIN32_LEAN_AND_MEAN //  �� Windows ͷ�ļ����ų�����ʹ�õ�����
#include <windows.h>  // ���� Windows API ͷ�ļ�
#endif

// ��� C++ ��׼�汾������汾 >= C++17������ tiny_cpp17 ��
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define tiny_cpp17
#endif

// ���� dllx �꣬���ڵ��� DLL ����
#if defined(WIN32)  // ����� Windows ƽ̨
#define dllx extern "C" __declspec(dllexport)  // ʹ�� __declspec(dllexport) ����
#elif defined(GNUC)  // ����� GCC ������
#define dllx extern "C" __attribute__((visibility("default")))  // ʹ�� visibility ���Ե���
#else  // ����ƽ̨
#define dllx extern "C"  // ֻʹ�� extern "C"
#endif

#define tiny_trace  // ���� trace ��������Ҫ user32.lib �� Kernel32.lib
// #define tiny_wtrace // ���� wtrace ��������Ҫ user32.lib �� Kernel32.lib
// #define tiny_memset  // ���� tiny_memset ����
#define tiny_memcpy  // ���� tiny_memcpy ����
// #define tiny_malloc  // ���� tiny_malloc ����

#ifdef tiny_trace  // ��������� trace ����
#ifdef _WINDOWS  // ����� Windows ƽ̨
void trace(const char* format, ...);  // ���� trace ����
#else  // ����ƽ̨
// ���� trace ������ʹ�� printf �����Ϣ
#define trace(...)                                                          \
  {                                                                          \
    printf("[TinyModInjector:%d] ", __LINE__);  \
    printf(__VA_ARGS__);                                                    \
    printf("\n");                                                            \
    fflush(stdout);                                                         \
  }
#endif
#endif

#ifdef tiny_wtrace  // ��������� wtrace ����
void wtrace(const wchar_t* format, ...);  // ���� wtrace ����
#endif

// typed memory helpers ����
#pragma region typed memory helpers

// �����ڴ沢ת��Ϊ T* ����
template <typename T>
T* malloc_arr(size_t count) {
    return (T*)malloc(sizeof(T) * count);
}

// ���·����ڴ沢ת��Ϊ T* ����
template <typename T>
T* realloc_arr(T* arr, size_t count) {
    return (T*)realloc(arr, sizeof(T) * count);
}

// �����ڴ沢ת��Ϊ T* ����
template <typename T>
T* memcpy_arr(T* dst, const T* src, size_t count) {
    return (T*)memcpy(dst, src, sizeof(T) * count);
}

#pragma endregion  // typed memory helpers �������

// TODO: reference additional headers your program requires here  // �����Ҫ������ͷ�ļ�