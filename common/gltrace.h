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

#include <map>
#include <vector>

class ContextState;
class FileLike;
class GLBuffer;
class GLFrameBufferObject;
class GLProgram;
class GLProgramARB;
class GLRenderBufferObject;
class GLSampler;
class GLShader;
class GLTexture;
struct SSerializeDataPacket;

class GLTrace
{
public:
	GLTrace();
	~GLTrace();	

	void Finalize();

	void Reset();
	void ReadContextState(FileLike* _from);
	void RecvGLCommand(const SSerializeDataPacket& _pkt);

	void Save(const char* _filename);
	static GLTrace* Load(const char* _filename);

	void CreateResources();
	void RestoreContextState();
	void BindResources();

	void Render();
	bool IsReplayComplete() const;

	ContextState* GetContextState() const { return mContextState; }
	
	// For a given handle from a trace, return the handle of that object to replay with.
	GLuint GetReplayBufferHandle(GLuint _traceBufferHandle) const;
	GLuint GetReplayFrameBufferObjectHandle(GLuint _traceFrameBufferObjectHandle) const;
	GLuint GetReplayProgramARBHandle(GLuint _traceProgramARBHandle) const;
	GLuint GetReplayProgramGLSLHandle(GLuint _traceProgramGLSLHandle) const;
	GLuint GetReplayRenderBuffer(GLuint _traceRenderBufferHandle) const;
	GLuint GetReplaySamplerHandle(GLuint _traceSamplerHandle) const;
	GLuint GetReplayShaderHandle(GLuint _traceShaderHandle) const;
	GLuint GetReplayTextureHandle(GLuint _traceTextureHandle) const;
	GLuint GetMaxTextureHandle() const;


	void glUseProgram(GLuint program) { mProgramGLSL = program; }

	inline GLint GetUniformLocation(GLint _traceLocation) 
	{
		// -1 is a safe value, per OGL spec.
		auto progIt = mUniformLocations.find(mProgramGLSL);
		if (progIt == mUniformLocations.end()) {
			return -1;
		}

		auto uniIt = progIt->second.find(_traceLocation);
		if (uniIt == progIt->second.end()) {
			return -1;
		}

		return uniIt->second;
	}
		
private:
	std::map<GLuint, GLuint> mTextureMap;
	std::map<GLuint, GLuint> mBufferMap;
	std::map<GLuint, GLuint> mShaderMap;
	std::map<GLuint, GLuint> mProgramMap;
	std::map<GLuint, GLuint> mProgramARBMap;
	std::map<GLuint, GLuint> mRenderBufferMap;
	std::map<GLuint, GLuint> mFrameBufferObjectMap;
	std::map<GLuint, std::map<GLint, GLint>> mUniformLocations;
	std::map<GLuint, GLuint> mSamplerObjectMap;

	GLuint mMaxTextureHandle;
	GLuint mProgramGLSL;

	ContextState* mContextState;
	std::vector<SSerializeDataPacket> mGLCommands;

	void CreateTexture(GLuint _traceTextureHandle, const GLTexture* _glTexture);
	void CreateBuffer(GLuint _traceBufferHandle, const GLBuffer* _glBuffer);
	void CreateShader(GLuint _traceHandle, const GLShader* _glShader);
	void CreateProgram(GLuint _traceHandle, const GLProgram* _glProgram);
	void CreateProgramARB(GLuint _traceHandle, const GLProgramARB* _glProgramARB);
	void CreateRenderBuffer(GLuint _traceHandle, const GLRenderBufferObject* _glRenderBuffer);
	void CreateFrameBufferObject(GLuint _traceHandle, const GLFrameBufferObject* _glFrameBufferObject);
	void CreateSamplerObject(GLuint _traceHandle, const GLSampler* _glSamplerObject);
};

void SetReplayTrace(GLTrace* _trace);
GLTrace* GetReplayTrace();
