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

GLuint gTexExamineNum = 1137;

class TextureViewer
{
public:
	TextureViewer()
	: mVS(0)
	, mFS(0)
	, mProg(0)
	, mVB(0)
	{
		LoadShaders();
		CreateVertexData();
	}

	~TextureViewer()
	{
		if (mFS) {
			glDeleteShader(mFS);
		}

		if (mVS) {
			glDeleteShader(mVS);
		}

		if (mProg) {
			glDeleteProgram(mProg);
		}

		if (mVB) {
			glDeleteBuffers(1, &mVB);
		}
	}

	void Render(GLuint _texture, GLint _w, GLint _h, GLint _left, GLint _top)
	{
		glUseProgram(mProg);

		glActiveTexture( GL_TEXTURE0 + mSamplerLoc);
		glBindTexture( GL_TEXTURE_2D, _texture );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		glEnableVertexAttribArray(mPosLoc);
		glEnableVertexAttribArray(mTexCoordLoc);
		glBindBuffer(GL_ARRAY_BUFFER, mVB);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (5 * sizeof(GLfloat)), (void*)(0 * sizeof(GLfloat)));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (5 * sizeof(GLfloat)), (void*)(3 * sizeof(GLfloat)));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(mTexCoordLoc);
		glDisableVertexAttribArray(mPosLoc);
	}
private:

	void LoadShaders()
	{
		const char* VertexShaderText = 
			"#version 140\n"
			"in vec3   aPosition;\n"
			"in vec2   aTexCoord;\n"
			"out vec2   vTexCoord;\n"
			"void main(void) {\n"
			"    gl_Position = vec4(aPosition.xyz, 1.0f);\n"
			"    vTexCoord = aTexCoord;\n"
			"}";

		mVS = CompileShader(GL_VERTEX_SHADER, VertexShaderText);
		const char* FragmentShaderText =
			"#version 140\n"
			"uniform sampler2D uColorMap;\n"
			"in vec2   vTexCoord;\n"
			"out vec4 oColor;\n"
			"void main(void) {\n"
			"    oColor = texture2D( uColorMap, vTexCoord.xy );\n"
			"}";
		mFS = CompileShader(GL_FRAGMENT_SHADER, FragmentShaderText);
		mProg = LinkProgram(mVS, mFS, "oColor");
		mPosLoc = glGetAttribLocation(mProg, "aPosition");
		mTexCoordLoc = glGetAttribLocation(mProg, "aTexCoord");
		mSamplerLoc = glGetUniformLocation(mProg, "uColorMap");
	}

	void CreateVertexData()
	{
		glGenBuffers(1, &mVB);
		glBindBuffer(GL_ARRAY_BUFFER, mVB);
		GLfloat triVerts[] = 
		{
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(triVerts), triVerts, GL_STATIC_DRAW);
	}

	GLuint mVS;
	GLuint mFS;
	GLuint mProg;
	GLuint mVB;

	GLuint mPosLoc;
	GLuint mTexCoordLoc;
	GLuint mSamplerLoc;
};

TextureViewer* gTextureViewer = NULL;

void Initialize()
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	gTextureViewer = new TextureViewer;
}

void Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
	gTextureViewer->Render(gTrace->GetReplayTextureHandle(gTexExamineNum), 0, 0, 0, 0);
}

void AdjustTextureExamined(int _adjFactor)
{
	GLint newTexNum = (GLint)((gTexExamineNum) + _adjFactor);
	GLint maxAllowedTraceTex = (GLint)gTrace->GetMaxTextureHandle();
	if (newTexNum < 0) {
		gTexExamineNum = maxAllowedTraceTex + newTexNum;
	} else if (newTexNum > maxAllowedTraceTex) {
		gTexExamineNum = newTexNum % maxAllowedTraceTex;
	} else { 
		gTexExamineNum = (GLuint)newTexNum;
	}
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

				case VK_HOME:
					gTexExamineNum = 1;
					break;

				case VK_END:
					gTexExamineNum = gTrace->GetMaxTextureHandle() - 1;
					break;

				default:
					break;
			}
            break;

		case WM_KEYDOWN:
			switch(wParam) 
			{
				case VK_PRIOR:
					AdjustTextureExamined(-1 * (lParam & 0xF));
					break;
			
				case VK_NEXT:
					AdjustTextureExamined(1 * (lParam & 0xF));
					break;

				default:
					break;
			}


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
    RECT rc = { 0, 0, 1024, 1024 };
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
		// return false;
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

	gTrace = GLTrace::Load(opts->OutputTraceName);
	gTrace->CreateResources();
	gTrace->RestoreContextState();

    MSG msg = {};
    while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			Render();
			SwapBuffers(dc);
		}
    }

	ReleaseDC(gHwnd, dc);

    DestroyWindow(gHwnd);
    UnregisterClass(L"WindowClass", NULL);

	SafeDelete(gTextureViewer);
	SafeDelete(gTrace);
	SafeDelete(opts);

    return (int)msg.wParam;
}
