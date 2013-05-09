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

class GLTrace;

enum GLBufferMapMode
{
	EUnmapped = 0,
	EMapBuffer,
	EMapBufferRange,

	GLBufferMapMode_MAX
};

// ------------------------------------------------------------------------------------------------
class GLBuffer
{
public:
	GLBuffer(GLenum _target=GL_NONE);
	~GLBuffer();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
	void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
	void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
	GLvoid* glMapBuffer(GLvoid* data, GLenum target, GLenum access);
	GLvoid* glMapBufferRange(GLvoid* data, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	GLboolean glUnmapBuffer(GLenum target);

	bool IsMapped() const { return mMapMode != EUnmapped; }

	GLuint Create(const GLTrace* _trace) const;

private:
	GLenum mTarget;

	// The real contents of the buffer, as far as we know.
	size_t mBufferSize;
	GLvoid* mBufferContents;

	GLenum mUsage;

	// Information for buffer maps.
	GLenum mMappedAccess;
	GLBufferMapMode mMapMode; 
	GLvoid* mDriverReturnedMappedPointer;
	size_t mMapOffset; // When doing MapBufferRange, remember how far into the buffer we mapped.

	size_t mMapSize;
	GLvoid* mFakeReturnedMappedPointer;
};
