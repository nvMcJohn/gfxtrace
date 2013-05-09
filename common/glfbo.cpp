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
#include "glfbo.h"
#include "gltrace.h"

#include "extensions.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLFrameBufferObjectAttachment::GLFrameBufferObjectAttachment()
: mAttachmentPoint(0)
, mAttachType(AT_Unknown)
, mAttachHandle(0)
, mAttachTarget(0)
, mAttachLevel(0)
, mAttachLayer(0)
{

}

// ------------------------------------------------------------------------------------------------
GLFrameBufferObjectAttachment::~GLFrameBufferObjectAttachment()
{

}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObjectAttachment::Write(FileLike* _out) const
{
	_out->Write(mAttachmentPoint);
	_out->Write((GLuint)mAttachType);
	_out->Write(mAttachHandle);
	_out->Write(mAttachTarget);
	_out->Write(mAttachLevel);
	_out->Write(mAttachLayer);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObjectAttachment::Read(FileLike* _in)
{
	_in->Read(&mAttachmentPoint);
	_in->Read((GLuint*)&mAttachType);
	_in->Read(&mAttachHandle);
	_in->Read(&mAttachTarget);
	_in->Read(&mAttachLevel);
	_in->Read(&mAttachLayer);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObjectAttachment::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	mAttachType = AT_RenderBuffer;
	mAttachmentPoint = attachment;
	mAttachHandle = renderbuffer;
	mAttachTarget = renderbuffertarget;
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObjectAttachment::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	mAttachType = AT_Texture2D;
	mAttachmentPoint = attachment;
	mAttachHandle = texture;
	mAttachTarget = textarget;
	mAttachLevel = level;
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObjectAttachment::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
	mAttachType = AT_Texture3D;
	mAttachmentPoint = attachment;
	mAttachHandle = texture;
	mAttachTarget = textarget;
	mAttachLevel = level;
	mAttachLayer = layer;
}

// ------------------------------------------------------------------------------------------------
GLuint GLFrameBufferObjectAttachment::Create(const GLTrace* _glTrace) const
{
	switch (mAttachType) {
		case AT_RenderBuffer:
			{
				GLuint replayHandle = _glTrace->GetReplayRenderBuffer(mAttachHandle);
				assert(replayHandle != 0);
				::glFramebufferRenderbuffer(GL_FRAMEBUFFER, mAttachmentPoint, mAttachTarget, replayHandle); 
				break;
			}
		case AT_Texture2D:
			{
				GLuint replayHandle = _glTrace->GetReplayTextureHandle(mAttachHandle);
				assert(replayHandle != 0);
				::glFramebufferTexture2D(GL_FRAMEBUFFER, mAttachmentPoint, mAttachTarget, replayHandle, mAttachLevel); 
				break;
			}
		case AT_Texture3D:
			{
				GLuint replayHandle = _glTrace->GetReplayTextureHandle(mAttachHandle);
				assert(replayHandle != 0);
				::glFramebufferTexture3D(GL_FRAMEBUFFER, mAttachmentPoint, mAttachTarget, replayHandle, mAttachLevel, mAttachLayer); 
				break;
			}
		default:
			assert(!"impl");
			return 0;
	}

	CHECK_GL_ERROR();

	return 1;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLFrameBufferObject::GLFrameBufferObject(GLuint _id)
: mFrameBufferObject(_id)
, mDrawBuffer(GL_NONE)
, mReadBuffer(GL_NONE)
, mIsComplete(false)
{

}

// ------------------------------------------------------------------------------------------------
GLFrameBufferObject::~GLFrameBufferObject()
{

}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::Write(FileLike* _out) const
{
	_out->Write(mFrameBufferObject);
	_out->Write(mAttachments);
	_out->Write(mDrawBuffer);
	_out->Write(mReadBuffer);
	_out->Write(mIsComplete);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::Read(FileLike* _in)
{
	_in->Read(&mFrameBufferObject);
	_in->Read(&mAttachments);
	_in->Read(&mDrawBuffer);
	_in->Read(&mReadBuffer);
	_in->Read(&mIsComplete);

}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::glDrawBuffer(GLenum buffer)
{
	mDrawBuffer = buffer;
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	mAttachments[attachment].glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	mAttachments[attachment].glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
	mAttachments[attachment].glFramebufferTexture3D(target, attachment, textarget, texture, level, layer);
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::glReadBuffer(GLenum buffer)
{
	mReadBuffer = buffer;
}

// ------------------------------------------------------------------------------------------------
void GLFrameBufferObject::OnDeleteRenderbufferObject(GLuint _rbo)
{
	// Check everywhere to see if it's bound--if so unbind it.

}

// ------------------------------------------------------------------------------------------------
GLuint GLFrameBufferObject::Create(const GLTrace* _glTrace) const
{
	GLuint retVal = 0;
	::glGenFramebuffers(1, &retVal);
	CHECK_GL_ERROR();

	::glBindFramebuffer(GL_FRAMEBUFFER, retVal);
	CHECK_GL_ERROR();

	for (auto it = mAttachments.cbegin(); it != mAttachments.cend(); ++it) {
		it->second.Create(_glTrace);
		CHECK_GL_ERROR();
	}

	if (mDrawBuffer != GL_NONE) {
		::glDrawBuffer(mDrawBuffer);
		CHECK_GL_ERROR();
	}

	if (mReadBuffer != GL_NONE) {
		::glReadBuffer(mReadBuffer);
		CHECK_GL_ERROR();
	}
	
	bool isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return retVal;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLRenderBufferObject::GLRenderBufferObject(GLenum _target, GLuint _id)
: mTarget(_target)
, mRenderBufferObject(_id)
{

}

// ------------------------------------------------------------------------------------------------
GLRenderBufferObject::~GLRenderBufferObject()
{

}

// ------------------------------------------------------------------------------------------------
void GLRenderBufferObject::Write(FileLike* _out) const
{
	_out->Write(mTarget);
	_out->Write(mRenderBufferObject);
	_out->Write(mSamples);
	_out->Write((GLuint)mInternalFormat);
	_out->Write(mWidth);
	_out->Write(mHeight);
}

// ------------------------------------------------------------------------------------------------
void GLRenderBufferObject::Read(FileLike* _in)
{
	_in->Read(&mTarget);
	_in->Read(&mRenderBufferObject);
	_in->Read(&mSamples);
	_in->Read((GLuint*)&mInternalFormat);
	_in->Read(&mWidth);
	_in->Read(&mHeight);
}

// ------------------------------------------------------------------------------------------------
void GLRenderBufferObject::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	glRenderbufferStorageMultisample(target, 0, internalformat, width, height);
}

// ------------------------------------------------------------------------------------------------
void GLRenderBufferObject::glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	mTarget = target;
	mSamples = samples;
	mInternalFormat = internalformat;
	mWidth = width;
	mHeight = height;
}


// ------------------------------------------------------------------------------------------------
GLuint GLRenderBufferObject::Create(const GLTrace* _glTrace) const
{
	CHECK_GL_ERROR();

	GLuint retVal = 0;
	::glGenRenderbuffers(1, &retVal);
	CHECK_GL_ERROR();

	assert(mTarget == GL_RENDERBUFFER);
	::glBindRenderbuffer(mTarget, retVal);
	CHECK_GL_ERROR();

	::glRenderbufferStorageMultisample(mTarget, mSamples, mInternalFormat, mWidth, mHeight);
	CHECK_GL_ERROR();
	return retVal;
}
