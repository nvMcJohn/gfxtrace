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

#include <stdio.h>
#include "thirdparty/mhook/mhook-lib/mhook.h"

// TODO: Move declarations to non-generated header
#include "common/functionhooks.gen.h"

extern bool gFirstMakeCurrent;

#pragma comment(lib, "opengl32.lib")

void AttachDetours()
{
	// If you need to debug startup, build with this set to true, then attach and change it to false.
	bool debugStartup = true;
	while (debugStartup) ;

	Generated_AttachStaticHooks();
}

void __cdecl TrapExit()
{
	int i = 0;
	++i;
}

void DetachHooks()
{
	// TODO: Get from code gen
	Generated_DetachAllHooks();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hModule;
	lpReserved;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			gMessageStream = new MessageStream(true, "", kPort);
			atexit(TrapExit);

			// TODO: This should come from the message stream.
			gOptions = new Options;
			gContextState = new ContextState;
			AttachDetours();
		}
		break;
	case DLL_PROCESS_DETACH:
		DetachHooks();
		SafeDelete(gContextState);
		SafeDelete(gMessageStream);
		SafeDelete(gOptions);
		gFirstMakeCurrent = true; // Need to re-find extensions if we detach.
		break;
	default:
		break;
	}
	return TRUE;
}
