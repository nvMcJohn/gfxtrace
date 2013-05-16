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

#include "stdafx.h"
#include "tracelog.h"

#include <list>
#include <vector>

#include "functionhooks.gen.h"

// Unfortunately, has to be stored as a global because GL is a C API. Boo hoo.
std::list<std::pair<TraceLogLevel, std::vector<TCHAR>>> mMessages;

void TraceGuts(TraceLogLevel _level, bool _log, const TCHAR* _fmt, va_list _args)
{
	// Doh, this is MS specific. Need a WAR for other platforms.
	size_t requiredLength = _vsctprintf(_fmt, _args) + 1;

	std::vector<TCHAR> buffer(requiredLength);
	_vsntprintf_s(&buffer[0], requiredLength, requiredLength - 1, _fmt, _args);

    if (_log) {
        PrintTraceMessage(_level, &(*buffer.begin()));
    } else {
	    mMessages.push_back(std::make_pair(_level, buffer));
    }
}

void WriteMessages(FileLike* _out)
{
	for (auto it = mMessages.begin(); it != mMessages.end(); ++it) {
		SSerializeDataPacket pkt;
		pkt.mDataType = EST_Message;
		pkt.mData_Message.level = it->first;
		pkt.mData_Message.messageBody = &it->second[0];

		_out->Write(pkt);
	}

	mMessages.clear();
}

void TraceVerbose(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLVerbose, false, fmt, args);
	va_end(args);
}

void TraceInfo(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLInfo, false, fmt, args);
	va_end(args);
}

void TraceWarn(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLWarn, false, fmt, args);
	va_end(args);
}

void TraceError(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLError, false, fmt, args);
	va_end(args);
}

void LogVerbose(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLVerbose, true, fmt, args);
	va_end(args);
}

void LogInfo(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLVerbose, true, fmt, args);
	va_end(args);
}

void LogWarn(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLVerbose, true, fmt, args);
	va_end(args);
}

void LogError(const TCHAR* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TraceGuts(TLLVerbose, true, fmt, args);
	va_end(args);
}

// To display the trace messages to stdout.
void PrintTraceMessage(int _level, const TCHAR* _body)
{
	const TCHAR* levelHeader = TC("Unknown");
	switch (_level) 
	{
		case TLLVerbose: levelHeader = TC("Verbose"); break;
		case TLLInfo: levelHeader = TC("Info"); break;
		case TLLWarn: levelHeader = TC("Warning"); break;
		case TLLError: levelHeader = TC("ERROR"); break;
		default:
			assert(0);
			break;
	};

	_tprintf(TC("%s: %s\n"), levelHeader, _body);
}
