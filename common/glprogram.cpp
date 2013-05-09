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
#include "common/glprogram.h"

#include <algorithm>
#include "extensions.h"
#include "functionhooks.gen.h"
#include "gltrace.h"

const GLint kUseStrlen = -1;

// ------------------------------------------------------------------------------------------------
static void AssertFunc()
{
	CompileTimeAssert(sizeof(GLchar) == sizeof(char));
}

// ------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
void update(std::map<T1, T2>* _dst, const std::map<T1, T2>& _src)
{
	for (auto it = _src.cbegin(); it != _src.cend(); ++it) {
		(*_dst)[it->first] = it->second;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLUniformVector::GLUniformVector()
: mType(UT_Typeless)
, mDimensions(0)
{
	memset(&mFloat.data[0], 0, sizeof(mFloat));
}

// ------------------------------------------------------------------------------------------------
GLUniformVector::~GLUniformVector()
{

}

// ------------------------------------------------------------------------------------------------
void GLUniformVector::Write(FileLike* _out) const
{
	_out->Write((unsigned int)mType);
	_out->Write(mDimensions);

	if (mType != UT_Typeless) {
		for (int i = 0; i < kEntriesPerVector; ++i) {
			_out->Write(mFloat.data[i]);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void GLUniformVector::Read(FileLike* _in)
{
	_in->Read((unsigned int*)&mType);
	_in->Read(&mDimensions);

	// Don't bother saving uninitialized uniforms--just a waste of space.
	if (mType != UT_Typeless) {
		for (int i = 0; i < kEntriesPerVector; ++i) {
			_in->Read(&mFloat.data[i]);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void GLUniformVector::Create(GLint _replayDest) const
{
	switch(mType)
	{
	case UT_Float:
		{
			switch (mDimensions)
			{
			case 1: 
				::glUniform1f(_replayDest, mFloat.data[0]);
				break;
			case 4:
				::glUniform4fv(_replayDest, 1, mFloat.data);
				break;
			default:
				assert(!"impl");
			};
			break;
		}

	case UT_Int:
		{
			switch (mDimensions)
			{
			case 1: 
				::glUniform1i(_replayDest, mInt.data[0]);
				break;
			case 4:
				::glUniform4iv(_replayDest, 1, mInt.data);
				break;
			default:
				assert(!"impl");
			};
			break;
		}
	default:
		assert(!"impl");
		break;
	}
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLShader::GLShader(GLenum _type, GLuint _shader)
: mType(_type)
, mShader(_shader)
, mShaderCompileStatus(SBS_Unbuilt)
, mAttachCount(0) 
, mMarkedForDelete(false)
{
	
}

// ------------------------------------------------------------------------------------------------
GLShader::~GLShader()
{

}

// ------------------------------------------------------------------------------------------------
void GLShader::Write(FileLike* _out) const
{
	_out->Write(mType);
	_out->Write(mShader);
	_out->Write((unsigned int)mShaderCompileStatus);
	_out->Write(mAttachCount);
	_out->Write(mMarkedForDelete);
	_out->Write(mShaderSourceStrings);
}

// ------------------------------------------------------------------------------------------------
void GLShader::Read(FileLike* _in)
{
	_in->Read(&mType);
	_in->Read(&mShader);
	_in->Read((unsigned int*)&mShaderCompileStatus);
	_in->Read(&mAttachCount);
	_in->Read(&mMarkedForDelete);
	_in->Read(&mShaderSourceStrings);
}

// ------------------------------------------------------------------------------------------------
void GLShader::glCompileShader(GLuint shader)
{
	assert(mShader == shader);

	// Record the shader compiler status 
	GLint glcompileStatus = 0;
	gReal_glGetShaderiv(shader, GL_COMPILE_STATUS, &glcompileStatus);
	mShaderCompileStatus = (glcompileStatus == GL_TRUE) ? SBS_Success : SBS_Failure;
}

// ------------------------------------------------------------------------------------------------
void GLShader::glDeleteShader(GLuint shader)
{
	assert(mShader == shader);
	mMarkedForDelete = true;
}

// ------------------------------------------------------------------------------------------------
void GLShader::glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	assert(mShader == shader);

	if (count == 0) {
		return;
	}

	// We always promote to null-terminated strings, cause it reduces the code a lot and this is
	// not a case where we need the exact conversation recorded properly--shader compilation is
	// expensive, but not because of one extra strlen in the driver.
	mShaderSourceStrings.resize(count);
	for (int i = 0; i < count; ++i) {
		if (length) {
			mShaderSourceStrings[i].assign(string[i], length[i]);
		} else {
			mShaderSourceStrings[i].assign(string[i]);
		}
	}
}

// ------------------------------------------------------------------------------------------------
GLuint GLShader::Create(const GLTrace* _trace) const
{
	if (mShaderSourceStrings.size() == 0) {
		// Possibly a bug.
		assert(0);
		return 0;
	}

	CHECK_GL_ERROR();


	GLuint returnHandle = ::glCreateShader(mType);
	CHECK_GL_ERROR();

	const char** strPointers = new const char*[mShaderSourceStrings.size()];
	for (size_t u = 0; u < mShaderSourceStrings.size(); ++u) {
		strPointers[u] = mShaderSourceStrings[u].c_str();
	}
	
	::glShaderSource(returnHandle, mShaderSourceStrings.size(), strPointers, NULL);
	CHECK_GL_ERROR();

	SafeDeleteArray(strPointers);

	::glCompileShader(returnHandle);
	CHECK_GL_ERROR();

	GLint glcompileStatus = 0;
	glGetShaderiv(returnHandle, GL_COMPILE_STATUS, &glcompileStatus);
	assert(mShaderCompileStatus == ((glcompileStatus == GL_TRUE) ? SBS_Success : SBS_Failure));

	return returnHandle;		
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLProgram::GLProgram(ContextState* _ctxState, GLuint program)
: mProgram(program)
, mCtxState(_ctxState)
{
	InitializeUniformState(_ctxState);
}

// ------------------------------------------------------------------------------------------------
GLProgram::~GLProgram()
{

}

// ------------------------------------------------------------------------------------------------
void GLProgram::Write(FileLike* _out) const
{
	_out->Write(mProgram);
	// No serialize: mCtxState
	_out->Write((unsigned int)mProgramLinkStatus);
	_out->Write(mAttachedShaders);
	_out->Write(mUniforms);
	_out->Write(mAttribBinds);
	_out->Write(mPendingAttribBinds);
	_out->Write(mUniformLocations);
}

// ------------------------------------------------------------------------------------------------
void GLProgram::Read(FileLike* _in)
{
	_in->Read(&mProgram);
	// No serialize: mCtxState
	_in->Read((unsigned int*)&mProgramLinkStatus);
	_in->Read(&mAttachedShaders);
	_in->Read(&mUniforms);
	_in->Read(&mAttribBinds);
	_in->Read(&mPendingAttribBinds);
	_in->Read(&mUniformLocations);
}

// ------------------------------------------------------------------------------------------------
void GLProgram::glAttachShader(GLuint program, GLuint shader)
{
	assert(mProgram == program);
	mAttachedShaders.push_back(shader);
}

// ------------------------------------------------------------------------------------------------
void GLProgram::glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	assert(mProgram == program);
	mPendingAttribBinds[std::string(name)] = index;
}

// ------------------------------------------------------------------------------------------------
bool GLProgram::glDetachShader(GLuint program, GLuint shader)
{
	assert(mProgram == program);

	auto findIt = std::find(mAttachedShaders.begin(), mAttachedShaders.end(), shader);
	if (findIt != mAttachedShaders.end()) {
		mAttachedShaders.erase(findIt);
		
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
GLint GLProgram::glGetUniformLocation(GLint _retVal, GLuint program, const GLchar* name)
{
	assert(mProgram == program);
	// Remember where this value went.
	mUniformLocations[name] = _retVal;

	return _retVal;
}

// ------------------------------------------------------------------------------------------------
void GLProgram::glLinkProgram(GLuint program)
{
	assert(mProgram == program);

	// Record the link status 
	GLint glLinkStatus = 0;
	gReal_glGetProgramiv(program, GL_LINK_STATUS, &glLinkStatus);
	mProgramLinkStatus = (glLinkStatus == GL_TRUE) ? SBS_Success : SBS_Failure;

	if (mProgramLinkStatus == SBS_Success) {
		update(&mAttribBinds, mPendingAttribBinds);
		mPendingAttribBinds.clear();
	}
}

// ------------------------------------------------------------------------------------------------
GLuint GLProgram::Create(const GLTrace* _trace, std::map<GLint, GLint>* _outUniformMapping) const
{
	std::vector<GLuint> realShaderHandles;
	for (auto it = mAttachedShaders.cbegin(); it != mAttachedShaders.cend(); ++it) {
		realShaderHandles.push_back(_trace->GetReplayShaderHandle(*it));
	}

	CHECK_GL_ERROR();

	GLuint returnHandle = ::glCreateProgram();
	CHECK_GL_ERROR();

	if (realShaderHandles.size()) {
		for (auto it = realShaderHandles.cbegin(); it != realShaderHandles.cend(); ++it) {
			::glAttachShader(returnHandle, (*it));
		}

		for (auto it = mAttribBinds.cbegin(); it != mAttribBinds.cend(); ++it) {
			::glBindAttribLocation(returnHandle, it->second, it->first.c_str());
		}

		::glLinkProgram(returnHandle);
		CHECK_GL_ERROR();
		// TODO: Useful error stuff.
		GLint glLinkStatus = 0;
		glGetProgramiv(returnHandle, GL_LINK_STATUS, &glLinkStatus);
		assert(mProgramLinkStatus == ((glLinkStatus == GL_TRUE) ? SBS_Success : SBS_Failure));

		for (auto it = mPendingAttribBinds.cbegin(); it != mPendingAttribBinds.cend(); ++it) {
			::glBindAttribLocation(returnHandle, it->second, it->first.c_str());
		}

		for (auto it = mUniformLocations.cbegin(); it != mUniformLocations.cend(); ++it) {
			(*_outUniformMapping)[it->second] = ::glGetUniformLocation(returnHandle, it->first.c_str());
		}

		::glUseProgram(returnHandle);

		// Now restore all of the uniform values
		GLint originalUniformLocation = 0;
		for (auto it = mUniforms.cbegin(); it != mUniforms.cend(); ++it, ++originalUniformLocation) {
			if (it->GetType() == UT_Typeless) {
				// Was never set, skip it.
				continue;
			}

			auto uniIt = (*_outUniformMapping).find(originalUniformLocation);

			// -1 is a safe place to stick data--it won't actually do anything.
			GLint replayLocation = -1;
			if (uniIt != (*_outUniformMapping).end()) {
				replayLocation = uniIt->second;
			}

			it->Create(replayLocation);
			CHECK_GL_ERROR();
		}
	}

	return returnHandle;	
}

// ------------------------------------------------------------------------------------------------
void GLProgram::InitializeUniformState(ContextState* _ctxState)
{
	// TODO: Still need to cache these somewhere. No need to ask for them every damn time. Kinda stupid.
	GLint fragmentUniformCount = 0;
	GLint vertexUniformCount = 0;

	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &fragmentUniformCount);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &vertexUniformCount);

	mUniforms.resize(fragmentUniformCount + vertexUniformCount);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLProgramARB::GLProgramARB(ContextState* _ctxState, GLuint program, GLenum target)
: mProgram(program)
, mTarget(target)
{

}

// ------------------------------------------------------------------------------------------------
GLProgramARB::~GLProgramARB()
{

}

// ------------------------------------------------------------------------------------------------
void GLProgramARB::Write(FileLike* _out) const
{
	_out->Write(mProgram);
	_out->Write(mTarget);
	_out->Write(mProgramString);
}

// ------------------------------------------------------------------------------------------------
void GLProgramARB::Read(FileLike* _in)
{
	_in->Read(&mProgram);
	_in->Read(&mTarget);
	_in->Read(&mProgramString);
}

// ------------------------------------------------------------------------------------------------
void GLProgramARB::glProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid* string)
{
	assert(CheckAndSetTarget(target));

	if (format != GL_PROGRAM_FORMAT_ASCII_ARB) {
		Once(TraceError("App is using glProgramStringARB with format != GL_PROGRAM_FORMAT_ASCII_ARB, which isn't currently handled."));
		return;
	}

	mProgramString.resize(len);
	memcpy(&mProgramString[0], string, len);
}

// ------------------------------------------------------------------------------------------------
bool GLProgramARB::CheckAndSetTarget(GLenum target)
{
	if (mTarget == 0) {
		mTarget = target;
		return true;
	}

	return mTarget == target;
}

// ------------------------------------------------------------------------------------------------
GLuint GLProgramARB::Create(const GLTrace* _trace) const
{
	CHECK_GL_ERROR();

	GLuint returnHandle = 0;
	::glGenProgramsARB(1, &returnHandle);

	if (mTarget) {
		// If mTarget is 0, then this program was never bound and so we shouldn't be able to do anything with it.
		// But it probably still needs a returnHandle, so go ahead and return one.
		CHECK_GL_ERROR();
	
		::glBindProgramARB(mTarget, returnHandle);
		CHECK_GL_ERROR();

		::glProgramStringARB(mTarget, GL_PROGRAM_FORMAT_ASCII_ARB, mProgramString.length(), mProgramString.c_str());
		CHECK_GL_ERROR();
		// TODO: ARB Program error checking. 
	}

	return returnHandle;	
}

