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

#include "common/common.h"
#include "common/gltrace.h"
#include "common/extensions.h"

#include "common/tracelog.h"

#pragma comment(lib, "opengl32.lib")

HWND gHwnd = 0;
GLTrace* gTrace = 0;

void Initialize()
{
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
	gTrace->Render();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_KEYUP:
			switch(wParam) 
			{
				case VK_ESCAPE: 
					PostQuitMessage(0);
					break;

				default:
					break;
			}
            break;

        case WM_NCHITTEST:
            return HTCAPTION;   // allows dragging of the window
		
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND InitWindow()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName =  L"WindowClass";
    RegisterClass(&wc);

    DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_VISIBLE;
    DWORD dwExStyle = 0;

    // Create window
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
    HWND hwnd = CreateWindowEx(dwExStyle, L"WindowClass", L"OpenGL", dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL);
    if (hwnd)
        ShowWindow(hwnd, SW_SHOW);

    return hwnd;
}

bool InitGL(HDC dc)
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_COPY;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cAlphaBits = 8;

    int pixelFormat = ChoosePixelFormat(dc, &pfd);
    if (pixelFormat == 0)
        return false;

    if (SetPixelFormat(dc, pixelFormat, &pfd) == FALSE)
        return false;

    HGLRC rc = wglCreateContext(dc);
    if (!rc)
        return false;

    if (wglMakeCurrent(dc, rc) == FALSE)
        return false;

	if (!ResolveExtensions()) {
		LogWarn(TC("Some extensions could not be initialized--playback may be incorrect."));
	}

	return true;
}

int CALLBACK WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
)
{
    gHwnd = InitWindow();
    if (gHwnd == NULL) {
        LogError(TC("Failed to create window..."));
        return 1;
    }

    HDC dc = GetDC(gHwnd);

    bool success = InitGL(dc);
    if (!success) {
        LogError(TC("Failed to initialize OpenGL..."));
        return 1;
    }

    Initialize();
	Options* opts = ParseCommandLine(0, NULL);

	SetReplayTrace(GLTrace::Load(opts->OutputTraceName));
	gTrace = GetReplayTrace();
	gTrace->CreateResources();
	gTrace->RestoreContextState();
	gTrace->BindResources();

    MSG msg = {};
    while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			if (!gTrace->IsReplayComplete()) {
				Render();
				SwapBuffers(dc);
			}
		}
    }

	ReleaseDC(gHwnd, dc);

    DestroyWindow(gHwnd);
    UnregisterClass(L"WindowClass", NULL);

	SafeDelete(gTrace);
	SafeDelete(opts);

    return (int)msg.wParam;
}
