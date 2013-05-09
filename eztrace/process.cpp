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
#include "process.h"

#include "thirdparty/mhook/mhook-lib/mhook.h"
#include "common/gltrace.h"

#include "common/functionhooks.gen.h"

#pragma comment(lib, "opengl32.lib")

const DWORD kPollTime = 250;

void SafeCloseHandle(HANDLE& _handle)
{
	if (_handle) {
		CloseHandle(_handle);
		_handle = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
DWORD WINAPI Process_RunWatchdogThread(LPVOID _processPtr)
{
	((Process*)_processPtr)->Thread_Watchdog();
	return 0;
}

// ------------------------------------------------------------------------------------------------
DWORD WINAPI Process_RunCaptureTraceThread(LPVOID _processPtr)
{
	((Process*)_processPtr)->Thread_CaptureTrace();

	return 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Process::Process(const TCHAR* _exeName, const TCHAR* _processArgs, const TCHAR* _workingDirectory, const char* _inceptionDllPath, GLTrace* _outTrace, const char* _outTraceFilename)
: mOutputTrace(_outTrace)
, mExeName(NULL)
, mProcessArgs(NULL)
, mWorkingDirectory(NULL)
, mInceptionDllPath(NULL)
, mWatchdogThreadHandle(NULL)
, mCaptureTraceThreadHandle(NULL)
, mParentThreadId(GetCurrentThreadId())
, mOutputTraceName(NULL)
, mServerRequestsTermination(false)
{
	mExeName = AllocateAndCopy(_exeName);
	mProcessArgs = AllocateAndCopy(_processArgs);
	mWorkingDirectory = AllocateAndCopy(_workingDirectory);
	mInceptionDllPath = AllocateAndCopy(_inceptionDllPath);
	mOutputTraceName = AllocateAndCopy(_outTraceFilename);

	memset(&mProcessInfo, 0, sizeof(mProcessInfo));

	SpawnInjectedProcess();
}

// ------------------------------------------------------------------------------------------------
Process::~Process()
{
	SafeCloseHandle(mCaptureTraceThreadHandle);
	SafeCloseHandle(mWatchdogThreadHandle);
	SafeDeleteArray(mOutputTraceName);
	SafeDeleteArray(mInceptionDllPath);
	SafeDeleteArray(mWorkingDirectory);
	SafeDeleteArray(mProcessArgs);
	SafeDeleteArray(mExeName);
}

// ------------------------------------------------------------------------------------------------
void Process::RunWatchdogThread()
{
	mWatchdogThreadHandle = CreateThread(NULL, 0, Process_RunWatchdogThread, this, 0, NULL);
}

// ------------------------------------------------------------------------------------------------
void Process::RunCaptureTraceThread()
{
	mCaptureTraceThreadHandle = CreateThread(NULL, 0, Process_RunCaptureTraceThread, this, 0, NULL);
}

// ------------------------------------------------------------------------------------------------
void Process::Start()
{
	ResumeThread(mProcessInfo.hThread);
}

// ------------------------------------------------------------------------------------------------
void Process::SpawnInjectedProcess()
{
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);

	DWORD processCreateFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	TCHAR fullExePath[_MAX_PATH];
	fullExePath[0] = 0;

    SetLastError(0);
	SearchPath(NULL, mExeName, TC(".exe"), ARRAYSIZE(fullExePath), fullExePath, NULL);

    if (!CreateProcess(fullExePath, mProcessArgs, NULL, NULL, TRUE, 
                       processCreateFlags, NULL, mWorkingDirectory,  
                       &si, &mProcessInfo)) {
        throw GetLastError();
    }

    size_t byteCount = strlen(mInceptionDllPath) + 1;
    // This is wasteful, but whatevs.
    void* targetProcessMem = VirtualAllocEx(mProcessInfo.hProcess, 0, byteCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!targetProcessMem) {
        throw 9;
    }

    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(mProcessInfo.hProcess, targetProcessMem, mInceptionDllPath, byteCount, &bytesWritten)) {
        throw bytesWritten;
    }

    HANDLE hRemoteThread = CreateRemoteThread(mProcessInfo.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) LoadLibraryA, 
                                              targetProcessMem, 0, NULL);

    if (!hRemoteThread) {
        throw GetLastError();
    }
}

// ------------------------------------------------------------------------------------------------
void Process::Thread_Watchdog()
{
	while (WaitForSingleObject(mProcessInfo.hProcess, kPollTime) == WAIT_TIMEOUT) {
		if (mServerRequestsTermination) {
			return;
		}
	}

	PostThreadMessage(mParentThreadId, WM_QUIT, 0, 0);
}

// ------------------------------------------------------------------------------------------------
void Process::Thread_CaptureTrace()
{
	while (1) {
		if (mServerRequestsTermination) {
			return;
		}

		size_t lastPacketCommand = (size_t)-1;
		int frameCommandCount = 0;
		
		SSerializeDataPacket lastPkt;
		FileLike fileLikeSocket(gMessageStream);

		try {
			// This means either we never started a capture or we've completed one. Either one is fine.
			fileLikeSocket.Read(Checkpoint("TraceCapturingBegin"));
		} catch (NetworkError &e)
		{
			if (e.IsConnectionReset()) {
				return;
			}
		}

		// Reset for a new frame capture.
		// TODO: This is currently destructive. I don't think it should be.
		mOutputTrace->Reset();

		mOutputTrace->ReadContextState(&fileLikeSocket);
		printf("Received Context State!\n");

		// TODO: Make this async again. 
		fileLikeSocket.Read(Checkpoint("FrameCommandsBegin"));
		printf("Beginning to collect frame commands...\n");
		// TODO: Receive the rest of the trace here!
		while (1) {
			SSerializeDataPacket pkt;
			fileLikeSocket.Read(&pkt);

			if (lastPacketCommand != (size_t)-1 && pkt.mPacketId != lastPacketCommand + 1) {
				printf("ERROR: We went out of sync with the application.\n");
				throw 8;
			}

			mOutputTrace->RecvGLCommand(pkt);
			if (pkt.mDataType == EST_Message) {
				PrintTraceMessage(pkt.mData_Message.level, pkt.mData_Message.messageBody);
			}
				
			// For debugging.
			lastPkt = pkt;
			// Book-keeping, also for debugging.
			lastPacketCommand = pkt.mPacketId;
			++frameCommandCount;

			// Once we get the sentinel, let's bail.
			if (pkt.mDataType == EST_Sentinel) {
				printf("Received %d frame commands\n", frameCommandCount);
				break;
			}
		}

		fileLikeSocket.Read(Checkpoint("FrameCommandsEnd"));
		fileLikeSocket.Read(Checkpoint("TraceCapturingEnd"));

		// TODO: get from command line options.
		printf("Saving capture to %s...\n", mOutputTraceName);
		mOutputTrace->Save(mOutputTraceName);

		printf("Frame successfully transfered.\n");
	}
}
