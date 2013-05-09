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

enum AttachmentType
{
	AT_Unknown = 0,
	AT_RenderBuffer,
	AT_Texture1D,
	AT_Texture2D,
	AT_Texture3D,
	AT_TextureLayer
};

// ------------------------------------------------------------------------------------------------
class GLFrameBufferObjectAttachment
{
public:
	GLFrameBufferObjectAttachment();
	~GLFrameBufferObjectAttachment();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);

	GLuint Create(const GLTrace* _glTrace) const;

private:
	GLenum mAttachmentPoint;

	AttachmentType mAttachType;
	GLuint mAttachHandle;
	GLuint mAttachTarget;
	GLuint mAttachLevel;
	GLuint mAttachLayer;
};

// ------------------------------------------------------------------------------------------------
class GLFrameBufferObject
{
public:
	GLFrameBufferObject(GLuint _id=0);
	~GLFrameBufferObject();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glDrawBuffer(GLenum buffer);
	void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
	void glReadBuffer(GLenum buffer);

	GLuint Create(const GLTrace* _glTrace) const;

	void OnDeleteRenderbufferObject(GLuint _rbo);
private:
	GLuint mFrameBufferObject;
	
	std::map<GLenum, GLFrameBufferObjectAttachment> mAttachments;
	GLenum mDrawBuffer;
	GLenum mReadBuffer;

	bool mIsComplete;
};

// ------------------------------------------------------------------------------------------------
class GLRenderBufferObject
{
public:
	GLRenderBufferObject(GLenum _target=GL_NONE, GLuint _id=0);
	~GLRenderBufferObject();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

	GLuint Create(const GLTrace* _glTrace) const;
private:
	GLuint mRenderBufferObject;
	GLenum mTarget;

	GLsizei mSamples;
	GLenum mInternalFormat;
	GLsizei mWidth;
	GLsizei mHeight;

};
