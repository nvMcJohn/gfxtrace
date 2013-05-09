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
#include "options.h"

Options* gOptions = NULL;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Options::Options()
{
	memset(this, 0, sizeof(*this));

	// Set defaults.
	strcpy_s(OutputTraceName, "D:\\tf2.glt");
	_tcscpy_s(ExeName, TC("D:\\p4\\valvesoftware\\console\\ValveGames\\game\\hl2.exe"));
	_tcscpy_s(ProcessArgs, TC("D:\\p4\\valvesoftware\\console\\ValveGames\\game\\hl2.exe -game tf -sw -dev -w 1280 -h 720"));
	_tcscpy_s(WorkingDirectory, TC("D:\\p4\\valvesoftware\\console\\ValveGames\\game"));
	
#ifdef _DEBUG
	GetFullPathNameA("..\\Debug\\inception.dll", ARRAYSIZE(InceptionDllPath), InceptionDllPath, NULL);
#else
	GetFullPathNameA("..\\Release\\inception.dll", ARRAYSIZE(InceptionDllPath), InceptionDllPath, NULL);
#endif

	ServerPort = 65536 - 31337;

	CaptureAllTextures = true;
	FixBadFlushBufferRangeArgs = true;
}

// ------------------------------------------------------------------------------------------------
Options* ParseCommandLine(int argc, TCHAR *argv[])
{
	Options* retVal = new Options;

	// TODO: Parse options.

	return retVal;
}

