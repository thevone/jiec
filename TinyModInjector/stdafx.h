#pragma once  // 确保头文件只被包含一次

#ifdef _WINDOWS  // 如果是 Windows 平台
#include "targetver.h"  // 包含目标平台版本信息

// #define WIN32_LEAN_AND_MEAN //  从 Windows 头文件中排除很少使用的内容
#include <windows.h>  // 包含 Windows API 头文件
#endif

// 检查 C++ 标准版本，如果版本 >= C++17，则定义 tiny_cpp17 宏
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define tiny_cpp17
#endif

// 定义 dllx 宏，用于导出 DLL 函数
#if defined(WIN32)  // 如果是 Windows 平台
#define dllx extern "C" __declspec(dllexport)  // 使用 __declspec(dllexport) 导出
#elif defined(GNUC)  // 如果是 GCC 编译器
#define dllx extern "C" __attribute__((visibility("default")))  // 使用 visibility 属性导出
#else  // 其他平台
#define dllx extern "C"  // 只使用 extern "C"
#endif

#define tiny_trace  // 启用 trace 函数，需要 user32.lib 和 Kernel32.lib
// #define tiny_wtrace // 启用 wtrace 函数，需要 user32.lib 和 Kernel32.lib
// #define tiny_memset  // 启用 tiny_memset 函数
#define tiny_memcpy  // 启用 tiny_memcpy 函数
// #define tiny_malloc  // 启用 tiny_malloc 函数

#ifdef tiny_trace  // 如果启用了 trace 函数
#ifdef _WINDOWS  // 如果是 Windows 平台
void trace(const char* format, ...);  // 声明 trace 函数
#else  // 其他平台
// 定义 trace 函数，使用 printf 输出信息
#define trace(...)                                                          \
  {                                                                          \
    printf("[TinyModInjector:%d] ", __LINE__);  \
    printf(__VA_ARGS__);                                                    \
    printf("\n");                                                            \
    fflush(stdout);                                                         \
  }
#endif
#endif

#ifdef tiny_wtrace  // 如果启用了 wtrace 函数
void wtrace(const wchar_t* format, ...);  // 声明 wtrace 函数
#endif

// typed memory helpers 区域
#pragma region typed memory helpers

// 分配内存并转换为 T* 类型
template <typename T>
T* malloc_arr(size_t count) {
    return (T*)malloc(sizeof(T) * count);
}

// 重新分配内存并转换为 T* 类型
template <typename T>
T* realloc_arr(T* arr, size_t count) {
    return (T*)realloc(arr, sizeof(T) * count);
}

// 复制内存并转换为 T* 类型
template <typename T>
T* memcpy_arr(T* dst, const T* src, size_t count) {
    return (T*)memcpy(dst, src, sizeof(T) * count);
}

#pragma endregion  // typed memory helpers 区域结束

// TODO: reference additional headers your program requires here  // 添加需要的其他头文件