#include "stdafx.h"  // ����Ԥ����ͷ�ļ�
#include <strsafe.h>  // �����ַ�����ȫ����

#if _WINDOWS  // ����� Windows ƽ̨
// ���� _fltused ����������֧�ָ�������
extern "C" int _fltused = 0;
#endif

#include <cstdint>  // ������׼�������Ͷ���

// ��������� tiny_trace ������ Windows ƽ̨
#if (defined(tiny_trace) && defined(_WINDOWS))
// trace �����������������Ϣ������̨
void trace(const char* pszFormat, ...) {  // pszFormat Ϊ��ʽ���ַ���
    char buf[1025];  // �洢��ʽ�����ַ����Ļ�����
    va_list argList;  // �ɱ�����б�
    va_start(argList, pszFormat);  // ��ʼ���ɱ�����б�
    wvsprintfA(buf, pszFormat, argList);  // ����ʽ���ַ���д�뻺����
    va_end(argList);  // ����ɱ�����б�
    DWORD done;  // ʵ��д����ֽ���
    auto len = strlen(buf);  // ��ȡ�ַ�������
    buf[len] = '\n';         // ��ӻ��з�
    buf[++len] = 0;          // ����ַ���������
    // ������������д���׼������
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, len, &done, NULL);
}
#endif

// ��������� tiny_memcpy ��
#ifdef tiny_memcpy
#pragma function(memcpy)  // ���߱�������Ҫ���� memcpy ����
// memcpy �������ڸ����ڴ��
void* memcpy(void* _Dst,   // Ŀ���ڴ���ַ
    const void* _Src,  // Դ�ڴ���ַ
    size_t _Size) {    // ���Ƶ��ֽ���
    auto src = static_cast<const uint8_t*>(_Src);  // ��Դ��ַת��Ϊ uint8_t* ����
    auto dst = static_cast<uint8_t*>(_Dst);        // ��Ŀ���ַת��Ϊ uint8_t* ����
    for (; _Size != 0; _Size--)  // ѭ������ÿ���ֽ�
        *dst++ = *src++;           // ��Դ�ֽڸ��Ƶ�Ŀ���ֽڣ�������ָ��
    return _Dst;                  // ����Ŀ���ڴ���ַ
}
#endif