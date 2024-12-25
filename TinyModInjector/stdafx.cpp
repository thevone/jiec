#include "stdafx.h"  // 包含预编译头文件
#include <strsafe.h>  // 包含字符串安全函数

#if _WINDOWS  // 如果是 Windows 平台
// 声明 _fltused 变量，用于支持浮点运算
extern "C" int _fltused = 0;
#endif

#include <cstdint>  // 包含标准整数类型定义

// 如果定义了 tiny_trace 宏且是 Windows 平台
#if (defined(tiny_trace) && defined(_WINDOWS))
// trace 函数用于输出调试信息到控制台
void trace(const char* pszFormat, ...) {  // pszFormat 为格式化字符串
    char buf[1025];  // 存储格式化后字符串的缓冲区
    va_list argList;  // 可变参数列表
    va_start(argList, pszFormat);  // 初始化可变参数列表
    wvsprintfA(buf, pszFormat, argList);  // 将格式化字符串写入缓冲区
    va_end(argList);  // 清理可变参数列表
    DWORD done;  // 实际写入的字节数
    auto len = strlen(buf);  // 获取字符串长度
    buf[len] = '\n';         // 添加换行符
    buf[++len] = 0;          // 添加字符串结束符
    // 将缓冲区内容写入标准输出句柄
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, len, &done, NULL);
}
#endif

// 如果定义了 tiny_memcpy 宏
#ifdef tiny_memcpy
#pragma function(memcpy)  // 告诉编译器不要内联 memcpy 函数
// memcpy 函数用于复制内存块
void* memcpy(void* _Dst,   // 目标内存块地址
    const void* _Src,  // 源内存块地址
    size_t _Size) {    // 复制的字节数
    auto src = static_cast<const uint8_t*>(_Src);  // 将源地址转换为 uint8_t* 类型
    auto dst = static_cast<uint8_t*>(_Dst);        // 将目标地址转换为 uint8_t* 类型
    for (; _Size != 0; _Size--)  // 循环复制每个字节
        *dst++ = *src++;           // 将源字节复制到目标字节，并递增指针
    return _Dst;                  // 返回目标内存块地址
}
#endif