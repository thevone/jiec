#include "stdafx.h"
#include <strsafe.h>

#if _WINDOWS
extern "C" int _fltused = 0;
#endif
#include <cstdint>

#if (defined(tiny_trace) && defined(_WINDOWS))
void trace(const char* pszFormat, ...) {
	char buf[1025];
	va_list argList;
	va_start(argList, pszFormat);
	wvsprintfA(buf, pszFormat, argList);
	va_end(argList);
	DWORD done;
	auto len = strlen(buf);
	buf[len] = '\n';
	buf[++len] = 0;
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, len, &done, NULL);
}
#endif

#ifdef tiny_memcpy
#pragma function(memcpy)
void* memcpy(void* _Dst, const void* _Src, size_t _Size) {
	auto src = static_cast<const uint8_t*>(_Src);
	auto dst = static_cast<uint8_t*>(_Dst);
	for (; _Size != 0; _Size--) *dst++ = *src++;
	return _Dst;
}
#endif