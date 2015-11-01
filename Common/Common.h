#pragma once

// ���ׂẴv���W�F�N�g�ɓK�p�����ǉ��w�b�_����ђ�`

#include <string>
#include <utility>
#include <map>
#include <vector>
#include <algorithm>
#include <tchar.h>
#include <windows.h>
#include <stdarg.h>

using std::string;
using std::wstring;
using std::pair;
using std::map;
using std::multimap;
using std::vector;

#undef NULL
#define NULL nullptr

template<class T> inline void SAFE_DELETE(T*& p) { delete p; p = NULL; }
template<class T> inline void SAFE_DELETE_ARRAY(T*& p) { delete[] p; p = NULL; }

inline void _OutputDebugString(const TCHAR* format, ...)
{
	// TODO: ���̊֐����͗\�񖼈ᔽ�̏�ɕ���킵���̂ŕύX���ׂ�
	va_list params;
	va_start(params, format);
	int length = _vsctprintf(format, params);
	va_end(params);
	if( length >= 0 ){
		TCHAR* buff = new TCHAR[length + 1];
		va_start(params, format);
		_vstprintf_s(buff, length + 1, format, params);
		va_end(params);
		OutputDebugString(buff);
		delete[] buff;
	}
}
