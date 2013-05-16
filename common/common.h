/*
 * Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "common/targetver.h"

#define WIN32_LEAN_AND_MEAN

#ifdef WIN32
#	include <Windows.h>
#endif

// TODO: This is backwards--should be MAC for the weird include.
#ifdef WIN32
#	include <gl/GL.h>
#else
#	include <gl.h>
#endif


#ifdef WIN32
#	include "wglext.h"
#else
#	include "common/glext.h"
#endif

#include "glext.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "common/options.h"
#include "common/interconnect.h"

template <typename T> void SafeDelete(T*& _ptr) { delete _ptr; _ptr = NULL; }
template <typename T> void SafeDeleteArray(T*& _arr) { delete [] _arr; _arr = NULL; }
template <typename T> void SafeDeleteArray(T**& _arr) { delete [] _arr; _arr = NULL; }

template <> 
inline void SafeDelete(struct addrinfo*& _addrInfo) 
{
	if (_addrInfo) {
		freeaddrinfo(_addrInfo);
		_addrInfo = NULL;
	}
}

template <typename T>
inline T* MallocAndCopy(const T* _src, size_t _len)
{
	T* retVal = (T*)malloc(_len);
	memcpy(retVal, _src, _len);
	return retVal;
}

template <typename T>
void SafeFree(T*& _ptr) { free(_ptr); _ptr = NULL; }

template <typename T>
void SafeFree(const T*& _ptr) { free(const_cast<T*>(_ptr)); _ptr = NULL; }

#define CompileTimeAssert(_asrt) \
	do { \
		switch( true ) { case false: break; case (_asrt): break; } \
	} while (0)


// if _val % _base != 0, rounds _val up to the next multiple of _base and returns it.
// Optionally returns the adjustment amount via _outAdjSize--if provided.
template <typename T>
T iceil(T _val, T _base, T* _outAdjSize = NULL)
{
	assert(_base > 0);
	T remainder = _val % _base;
	if (remainder != 0) {
		_val += (_base - remainder);
		if (_outAdjSize) {
			(*_outAdjSize) = (_base - remainder);
		}
	}

	return _val;
}

#ifdef _UNICODE
#	define TC(_x) L##_x
#else
#	define TC(_x) _x
#endif

inline char* AllocateAndCopy(const char* _src)
{
	size_t bufferSize = 1 + strlen(_src);
	
	char* retVal = new char[bufferSize];
	strcpy_s(retVal, bufferSize, _src);

	return retVal;
}

inline wchar_t* AllocateAndCopy(const wchar_t* _src)
{
	size_t bufferSize = 1 + wcslen(_src);
	
	wchar_t* retVal = new wchar_t[bufferSize];
	wcscpy_s(retVal, bufferSize, _src);

	return retVal;
}

inline TCHAR* AllocateAndCopyN(const TCHAR* _src, int _count)
{
	size_t bufferSize = 1 + _count;
	
	TCHAR* retVal = new TCHAR[bufferSize];
	_tcsncpy_s(retVal, bufferSize, _src, _count);

	return retVal;
}


inline void CHECK_GL_ERROR()
{
	GLenum err = ::glGetError();
	assert(err == 0);
}

#include "common/filelike.h"
#include "common/tracelog.h"
