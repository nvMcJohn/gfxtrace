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
#include "hotkey.h"

#include "common/gltrace.h"
#include "process.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OnHotkeyPressed()
{
	gMessageStream->Send(&RC_Capture(), sizeof(RC_Capture));
}

// ------------------------------------------------------------------------------------------------
void MainLoop(HotkeyManager* _hotkeyMgr)
{
	MSG msg = { 0 };
	bool quit = false;
	while (!quit) {
		quit = GetMessage(&msg, NULL, 0, 0) == 0 || quit;
		if (msg.message == WM_HOTKEY) {
			_hotkeyMgr->OnWmHotkey(msg);
		} 
	}
}

// ------------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	Options* opts = ParseCommandLine(argc, argv);

	HotkeyManager hotkeyManager(kIdStartRangeExe);
	hotkeyManager.AddHotkey(NULL, MOD_ALT | MOD_CONTROL | MOD_NOREPEAT, 'P', OnHotkeyPressed);
	
	GLTrace outputTrace;

	// Create and start the process.
	Process proc(opts->ExeName, opts->ProcessArgs, opts->WorkingDirectory, opts->InceptionDllPath, &outputTrace, opts->OutputTraceName);
	proc.Start();

	// Now connect the socket to the process.
	gMessageStream = new MessageStream(false, "127.0.0.1", kPort);

	// Start the capture trace thread, which will (duh) capture the trace for us.
	proc.RunCaptureTraceThread();

	// Start the watchdog thread, which will let us know if the process terminates before we do.
	proc.RunWatchdogThread();

	// Now into the main loop, listen for hotkeys to send over.
	MainLoop(&hotkeyManager);

	gMessageStream->OptionalSend(RC_Terminate());

	// Then cleanup time.
	SafeDelete(gMessageStream);
	outputTrace.Finalize();

	SafeDelete(opts);

	return 0;
}

