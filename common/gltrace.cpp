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
#include "gltrace.h"

#include "common/functionhooks.gen.h"
#include "common/extensions.h"

const unsigned int kEndianTestValue = 0x12345678;

GLTrace* gReplayTrace = NULL;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLTrace::GLTrace()
: mContextState(NULL)
, mMaxTextureHandle(0)
, mProgramGLSL(0)
{
	mContextState = new ContextState;
	gContextState = mContextState;
}

// ------------------------------------------------------------------------------------------------
GLTrace::~GLTrace()
{
	gContextState = NULL;
	SafeDelete(mContextState);

}

// ------------------------------------------------------------------------------------------------
void GLTrace::Finalize()
{

}

// ------------------------------------------------------------------------------------------------
void GLTrace::Reset()
{
	SafeDelete(mContextState);
	mContextState = new ContextState;
	
	// Set the global state
	gContextState = mContextState;

	// TODO: This leaks--need to actually free all of the memory in these commands.
	mGLCommands.clear();
}

// ------------------------------------------------------------------------------------------------
void GLTrace::ReadContextState(FileLike *_from)
{
	_from->Read(mContextState);
}

// ------------------------------------------------------------------------------------------------
void GLTrace::RecvGLCommand(const SSerializeDataPacket& _pkt)
{
	mGLCommands.push_back(_pkt);
}

// ------------------------------------------------------------------------------------------------
void GLTrace::Save(const char* _filename)
{
	FILE* wfp = 0;
	if (fopen_s(&wfp, _filename, "wb") != 0) {
		throw 10;
	}
	assert(wfp);

	{
		FileLike out(wfp);

		out.Write(Checkpoint("GLTrace"));
		unsigned int endianCheck = kEndianTestValue;
		out.Write(endianCheck); // To deal with endianness.

		// TODO: Should probably write out some metadata like resolution, extensions used, errors encountered, etc.

		out.Write(*mContextState);
		out.Write(mGLCommands);
	}

	fclose(wfp);
}

// ------------------------------------------------------------------------------------------------
GLTrace* GLTrace::Load(const char* _filename)
{
	FILE* rfp = 0;
	if (fopen_s(&rfp, _filename, "rb") != 0) {
		throw 10;
	}
	assert(rfp);

	GLTrace *retTrace = new GLTrace;
	{
		FileLike in(rfp);
		
		in.Read(Checkpoint("GLTrace"));
		unsigned int endianCheck;
		in.Read(&endianCheck);
		assert(kEndianTestValue == endianCheck);

		in.Read(retTrace->mContextState);
		in.Read(&(retTrace->mGLCommands));
	}
	fclose(rfp);

	return retTrace;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateResources()
{
	CHECK_GL_ERROR();

	// TODO: Other resources.
	const auto& textures = mContextState->GetTextureObjects();
	for (auto it = textures.cbegin(); it != textures.cend(); ++it) {
		CreateTexture(it->first, it->second);
	}

	const auto& buffers = mContextState->GetBufferObjects();
	for (auto it = buffers.cbegin(); it != buffers.cend(); ++it) {
		CreateBuffer(it->first, it->second);
	}

	const auto& shaders = mContextState->GetShaderObjectsGLSL();
	for (auto it = shaders.cbegin(); it != shaders.cend(); ++it) {
		CreateShader(it->first, it->second);
	}

	const auto& programs = mContextState->GetProgramObjectsGLSL();
	for (auto it = programs.cbegin(); it != programs.cend(); ++it) {
		CreateProgram(it->first, it->second);
	}

	const auto& programsARB = mContextState->GetProgramObjectsARB();
	for (auto it = programsARB.cbegin(); it != programsARB.cend(); ++it) {
		CreateProgramARB(it->first, it->second);
	}

	const auto& renderbuffers = mContextState->GetRenderBufferObjects();
	for (auto it = renderbuffers.cbegin(); it != renderbuffers.cend(); ++it) {
		CreateRenderBuffer(it->first, it->second);
	}

	const auto& fbos = mContextState->GetFrameBufferObjects();
	for (auto it = fbos.cbegin(); it != fbos.cend(); ++it) {
		CreateFrameBufferObject(it->first, it->second);
	}

	const auto& samplers = mContextState->GetSamplerObjects();
	for (auto it = samplers.cbegin(); it != samplers.cend(); ++it) {
		CreateSamplerObject(it->first, it->second);
	}
	

	CHECK_GL_ERROR();
}

// ------------------------------------------------------------------------------------------------
void GLTrace::RestoreContextState()
{
	mContextState->Restore();
}

// ------------------------------------------------------------------------------------------------
void GLTrace::BindResources()
{
	// Textures
	for (auto it = mContextState->mData_TextureUnits.cbegin(); it != mContextState->mData_TextureUnits.cend(); ++it) {
		::glActiveTexture(it->first.first);
		CHECK_GL_ERROR();
		
		if (it->second == 0) {
			continue;
		}

		GLuint replayTex = GetReplayTextureHandle(it->second);
		assert(replayTex);
		::glBindTexture(it->first.second, replayTex);
		CHECK_GL_ERROR();
	}

	// Restore the correct active texture unit
	::glActiveTexture(mContextState->mData_glActiveTexture.texture);
	CHECK_GL_ERROR();

	// PixelStore / PixelTransfer state.
	mContextState->GetPixelStoreState().Set();
	CHECK_GL_ERROR();

	mContextState->GetPixelTransferState().Set();
	CHECK_GL_ERROR();

	// Buffers
	for (auto it = mContextState->mData_BufferBindings.cbegin(); it != mContextState->mData_BufferBindings.cend(); ++it) {
		if (it->second == 0) {
			continue;
		}

		GLuint replayBuffer = GetReplayBufferHandle(it->second);
		::glBindBuffer(it->first, replayBuffer);
		CHECK_GL_ERROR();
	}

	// Programs
	if (mContextState->mData_glUseProgram.program) {
		GLuint replayProgram = GetReplayProgramGLSLHandle(mContextState->mData_glUseProgram.program);
		assert(replayProgram);
		::glUseProgram(replayProgram);
		CHECK_GL_ERROR();
	}

	// ProgramARB
	// TODO: Needs to be multi-state.

	// RenderBuffers
	const auto& rbBindings = mContextState->GetRenderBufferBindings();
	for (auto it = rbBindings.cbegin(); it != rbBindings.cend(); ++it) {
		if (it->second == 0) {
			continue;
		}

		GLuint replayRB = GetReplayRenderBuffer(it->second);
		assert(replayRB);
		::glBindRenderbuffer(it->first, replayRB);
		CHECK_GL_ERROR();
	}

	// FBOs
	::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK_GL_ERROR();
	const auto& fboBindings = mContextState->GetFrameBufferBindings();
	for (auto it = fboBindings.cbegin(); it != fboBindings.cend(); ++it) {
		if (it->second == 0) {
			continue;
		}

		GLuint replayFBO = GetReplayFrameBufferObjectHandle(it->second);
		assert(replayFBO);
		::glBindFramebuffer(it->first, replayFBO);
		CHECK_GL_ERROR();
	}

	// Sampler
	CHECK_GL_ERROR();
	const auto& samplerBindings = mContextState->GetSamplerBindings();
	for (auto it = samplerBindings.cbegin(); it != samplerBindings.cend(); ++it) {
		if (it->second == 0) {
			// Sampler 0 objects need to be set.
			::glBindSampler(it->first, it->second);
			continue;
		}

		GLuint replaySampler = GetReplaySamplerHandle(it->second);
		assert(replaySampler);
		::glBindSampler(it->first, replaySampler);
		CHECK_GL_ERROR();
	}

	CHECK_GL_ERROR();
}

// ------------------------------------------------------------------------------------------------
void GLTrace::Render()
{
	CHECK_GL_ERROR();

	int commandNum = 0;
	for (auto it = mGLCommands.cbegin(); it != mGLCommands.cend(); ++it) {
		it->Play();
		CHECK_GL_ERROR();
		++commandNum;
	}
}

// ------------------------------------------------------------------------------------------------
bool GLTrace::IsReplayComplete() const
{
	return false;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayTextureHandle(GLuint _traceTextureHandle) const
{
	auto it = mTextureMap.find(_traceTextureHandle);
	if (it == mTextureMap.end()) 
		return 0;
	return it->second;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetMaxTextureHandle() const
{
	return mMaxTextureHandle;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayBufferHandle(GLuint _traceBufferHandle) const
{
	auto it = mBufferMap.find(_traceBufferHandle);
	if (it == mBufferMap.end()) {
		return 0;
	}
	return it->second;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayProgramARBHandle(GLuint _traceProgramARBHandle) const
{
	auto it = mProgramARBMap.find(_traceProgramARBHandle);
	if (it == mProgramARBMap.end()) {
		return 0;
	}
	return it->second;	
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayProgramGLSLHandle(GLuint _traceProgramGLSLHandle) const
{
	auto it = mProgramMap.find(_traceProgramGLSLHandle);
	if (it == mProgramMap.end()) {
		return 0;
	}
	return it->second;	
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayRenderBuffer(GLuint _traceRenderBufferHandle) const
{
	auto it = mRenderBufferMap.find(_traceRenderBufferHandle);
	if (it == mRenderBufferMap.end()) {
		return 0;
	}
	return it->second;		
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayShaderHandle(GLuint _traceShaderHandle) const
{
	auto it = mShaderMap.find(_traceShaderHandle);
	if (it == mShaderMap.end()) {
		return 0;
	}
	return it->second;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplayFrameBufferObjectHandle(GLuint _traceFrameBufferObjectHandle) const
{
	auto it = mFrameBufferObjectMap.find(_traceFrameBufferObjectHandle);
	if (it == mFrameBufferObjectMap.end()) {
		return 0;
	}
	return it->second;
}

// ------------------------------------------------------------------------------------------------
GLuint GLTrace::GetReplaySamplerHandle(GLuint _traceHandle) const
{
	auto it = mSamplerObjectMap.find(_traceHandle);
	if (it == mSamplerObjectMap.end()) {
		return 0;
	}
	return it->second;
}


// ------------------------------------------------------------------------------------------------
void GLTrace::CreateTexture(GLuint _traceTextureHandle, const GLTexture* _glTexture)
{
	GLuint myTexHandle = _glTexture->Create(this);
	mTextureMap[_traceTextureHandle] = myTexHandle;

	if (_traceTextureHandle > mMaxTextureHandle) {
		mMaxTextureHandle = _traceTextureHandle;
	}
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateBuffer(GLuint _traceBufferHandle, const GLBuffer* _glBuffer)
{
	GLuint myBuffHandle = _glBuffer->Create(this);
	mBufferMap[_traceBufferHandle] = myBuffHandle;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateShader(GLuint _traceHandle, const GLShader* _glShader)
{
	GLuint myHandle = _glShader->Create(this);
	mShaderMap[_traceHandle] = myHandle;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateProgram(GLuint _traceHandle, const GLProgram* _glProgram)
{
	std::map<GLint, GLint> uniformMapping;
	GLuint myHandle = _glProgram->Create(this, &uniformMapping);
	mProgramMap[_traceHandle] = myHandle;
	
	// Also remember the mapping for uniforms
	mUniformLocations[myHandle] = uniformMapping;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateProgramARB(GLuint _traceHandle, const GLProgramARB* _glProgramARB)
{
	GLuint myHandle = _glProgramARB->Create(this);
	mProgramARBMap[_traceHandle] = myHandle;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateRenderBuffer(GLuint _traceHandle, const GLRenderBufferObject* _glRenderBuffer)
{
	GLuint myHandle = _glRenderBuffer->Create(this);
	mRenderBufferMap[_traceHandle] = myHandle;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateFrameBufferObject(GLuint _traceHandle, const GLFrameBufferObject* _glFrameBufferObject)
{
	GLuint myHandle = _glFrameBufferObject->Create(this);
	mFrameBufferObjectMap[_traceHandle] = myHandle;
}

// ------------------------------------------------------------------------------------------------
void GLTrace::CreateSamplerObject(GLuint _traceHandle, const GLSampler* _glSamplerObject)
{
	GLuint myHandle = _glSamplerObject->Create(this);
	mSamplerObjectMap[_traceHandle] = myHandle;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SetReplayTrace(GLTrace* _trace)
{
	gReplayTrace = _trace;
}

// ------------------------------------------------------------------------------------------------
GLTrace* GetReplayTrace()
{
	return gReplayTrace;
}
