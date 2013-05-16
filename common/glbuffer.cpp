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
#include "glbuffer.h"

#include "extensions.h"
#include "functionhooks.gen.h"

// ------------------------------------------------------------------------------------------------
static void DummyFunction()
{
	// If this fails, need to add an explicit GLbitfield field in GLBuffer for access when a buffer 
	// is mapped with mapbufferrange.
	CompileTimeAssert(sizeof(GLbitfield) == sizeof(GLenum));
}

// ------------------------------------------------------------------------------------------------
void* aligned_malloc(size_t _alignment, size_t _allocSize)
{
	// Code as written only uses a byte to specify where we need to back up to for the "real" 
	// pointer
	assert(_alignment < 256);
	
	unsigned char* realPtr = (unsigned char*) malloc(_allocSize + _alignment);
	if (realPtr == NULL) {
		return NULL;
	}

	size_t offset = 0;
	unsigned char* clientPtr = (unsigned char*) iceil(size_t(realPtr + 1), _alignment, &offset);
	offset += 1;

	// Store our corrective factor for later.
	clientPtr[-1] = offset;

	return clientPtr;
}

// ------------------------------------------------------------------------------------------------
void aligned_free(void* _ptr)
{
	if (_ptr) {
		unsigned char* clientPtr = (unsigned char*)_ptr;
		void* realPtr = &clientPtr[-clientPtr[-1]];
				
		free(realPtr);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLBuffer::GLBuffer(GLenum _target)
: mTarget(_target)
, mBufferSize(0)
, mBufferContents(NULL)
, mUsage(GL_STREAM_DRAW)
, mMappedAccess(GL_READ_WRITE)
, mMapMode(EUnmapped)
, mDriverReturnedMappedPointer(NULL)
, mMapOffset(0)
, mMapSize(0)
, mFakeReturnedMappedPointer(NULL)
{

}

// ------------------------------------------------------------------------------------------------
GLBuffer::~GLBuffer()
{
	SafeFree(mFakeReturnedMappedPointer);
	// Do not free mDriverReturnedMappedPointer, because we don't own it.
	SafeFree(mBufferContents);
	mTarget = GL_NONE;
}

// ------------------------------------------------------------------------------------------------
void GLBuffer::Write(FileLike* _out) const
{
	_out->Write(mTarget);
	_out->Write(mBufferContents, mBufferSize);
	_out->Write(mMappedAccess);
	_out->Write((unsigned int)mMapMode);
	// Never write out mDriverReturnedMappedPointer--there's no circumstances where it is useful 
	// to the replayer, and reading from it could cause Bad Things (TM) to happen.

	_out->Write(mMapSize);
	if (mFakeReturnedMappedPointer && (mFakeReturnedMappedPointer != mDriverReturnedMappedPointer)) {
		_out->Write(mFakeReturnedMappedPointer, mMapSize);
	} else { 
		_out->Write(NULL, 0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLBuffer::Read(FileLike* _in)
{
	_in->Read(&mTarget);
	_in->Read(&mBufferContents, &mBufferSize);
	_in->Read(&mMappedAccess);
	_in->Read((unsigned int*)&mMapMode);
	_in->Read(&mMapSize);
	
	// Set this to NULL. See above for why. 
	mDriverReturnedMappedPointer = NULL;

	size_t bufferSize = 0;
	_in->Read(&mFakeReturnedMappedPointer, &bufferSize);
	// Either of these are okay, but anything else suggests the stream is questionable.
	assert(bufferSize == 0 || bufferSize == mMapSize);
}

// ------------------------------------------------------------------------------------------------
void GLBuffer::glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	// Not sure I need the target this time. 
	mTarget = target;
	mBufferSize = size;
	mUsage = usage;
	
	// Cleanup after ourself in case of repeated calls.
	SafeFree(mBufferContents);

	if (data) {
		mBufferContents = MallocAndCopy(data, size);
	} else {
		// If not, our junk is as good as their junk.
		mBufferContents = malloc(size);
		assert(mBufferContents);
	}
}

// ------------------------------------------------------------------------------------------------
void GLBuffer::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
	assert(mTarget == target);
	assert(unsigned(offset + size) <= mBufferSize);

	memcpy((GLubyte*)mBufferContents + offset, data, size);
}

// ------------------------------------------------------------------------------------------------
void GLBuffer::glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	// Caller should've bailed in these cases.
	assert(offset >= 0);
	assert(length >= 0);
	assert(unsigned(offset + length) <= mMapSize);
	assert(unsigned(offset + length + mMapOffset) <= mBufferSize);

	if (offset == 0 && length == 0) {
		if (gOptions->FixBadFlushBufferRangeArgs) {
			length = mMapSize;
		}
		Once(TraceWarn(TC("Application issued glFlushMappedBufferRange with offset and length == 0--likely application bug.")));
	} 

	// Need to copy into our own version for consistency, and to the driver's copy because otherwise the 
	// change won't actually happen.
	if (length && mFakeReturnedMappedPointer != mDriverReturnedMappedPointer) {
		memcpy((GLubyte*)mBufferContents + mMapOffset + offset, (GLubyte*)mFakeReturnedMappedPointer + offset, length);
		memcpy((GLubyte*)mDriverReturnedMappedPointer + offset, (GLubyte*)mFakeReturnedMappedPointer + offset, length);
	}
}

// ------------------------------------------------------------------------------------------------
GLvoid* GLBuffer::glMapBuffer(GLvoid* data, GLenum target, GLenum access)
{
	if (!data) {
		return NULL;
	}

	mMapMode = EMapBufferRange;
	mDriverReturnedMappedPointer = data;
	mMapOffset = 0;
	mMapSize = mBufferSize;
	mMappedAccess = access;

	bool createFakeBuffer = ((mMappedAccess & GL_MAP_WRITE_BIT) != 0);
	if (createFakeBuffer) {
		// If they're going to write into the buffer, then we need to create a copy for them to scribble into
		// so we can keep track of what goes back to the driver.
		// TODO: Store this in the context or something.
		GLint alignment = 0;
		gReal_glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &alignment);
		
		mFakeReturnedMappedPointer = aligned_malloc(max(1, alignment), mMapSize);
		assert(mFakeReturnedMappedPointer);
		
		// Even if they can't read it, in case they don't write the whole thing.
		// This isn't striiiiictly correct, but it can avoid an app bug or two.
		memcpy(mFakeReturnedMappedPointer, (GLubyte*)mBufferContents, mMapSize);
	} else {
		mFakeReturnedMappedPointer = mDriverReturnedMappedPointer;
	}

	return mFakeReturnedMappedPointer;
}

// ------------------------------------------------------------------------------------------------
GLvoid* GLBuffer::glMapBufferRange(GLvoid* data, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	if (!data) {
		return NULL;
	}

	mMapMode = EMapBufferRange;
	mDriverReturnedMappedPointer = data;
	mMapOffset = offset;
	mMapSize = length;
	mMappedAccess = access;

	bool createFakeBuffer = ((mMappedAccess & GL_MAP_WRITE_BIT) != 0);

	if (createFakeBuffer) {
		// If they're going to write into the buffer, then we need to create a copy for them to scribble into
		// so we can keep track of what goes back to the driver.
		// TODO: Store this in the context or something.
		GLint alignment = 0;
		gReal_glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &alignment);
		
		mFakeReturnedMappedPointer = aligned_malloc(max(1, alignment), mMapSize);
		assert(mFakeReturnedMappedPointer);
		
		// Even if they can't read it, in case they don't write the whole thing.
		// This isn't striiiiictly correct, but it can avoid an app bug or two.
		memcpy(mFakeReturnedMappedPointer, (GLubyte*)mBufferContents + mMapOffset, mMapSize);
	} else {
		mFakeReturnedMappedPointer = mDriverReturnedMappedPointer;
	}

	return mFakeReturnedMappedPointer;
}

// ------------------------------------------------------------------------------------------------
GLboolean GLBuffer::glUnmapBuffer(GLenum target)
{
	if (mMapMode == EUnmapped) {
		return GL_FALSE;
	}

	bool copyFromFakeBuffer = false;
	if (mMapMode == EMapBuffer) {
		copyFromFakeBuffer = (mMappedAccess == GL_READ_WRITE || mMappedAccess == GL_WRITE_ONLY);
	} else if (mMapMode == EMapBufferRange) {
		copyFromFakeBuffer = ((mMappedAccess & GL_MAP_WRITE_BIT) != 0) && ((mMappedAccess & GL_MAP_FLUSH_EXPLICIT_BIT) == 0);
	} else {
		// Wut.
		assert(0);
	}
	
	if (copyFromFakeBuffer) {
		assert(mBufferContents);
		assert(mFakeReturnedMappedPointer);
		assert(mDriverReturnedMappedPointer);
		assert(mFakeReturnedMappedPointer != mDriverReturnedMappedPointer);

		memcpy((GLubyte*)mBufferContents + mMapOffset, mFakeReturnedMappedPointer, mMapSize);
		memcpy(mDriverReturnedMappedPointer, mFakeReturnedMappedPointer, mMapSize);
	}

	if (mFakeReturnedMappedPointer != mDriverReturnedMappedPointer) {
		aligned_free(mFakeReturnedMappedPointer);
	}

	mFakeReturnedMappedPointer = NULL;
	mDriverReturnedMappedPointer = NULL;
	mMapOffset = 0;
	mMapSize = 0;
	mMapMode = EUnmapped;

	// Don't actually know here, just return that it worked. Caller is gonna ignore this anyways.
	return GL_TRUE;
}

// ------------------------------------------------------------------------------------------------
GLuint GLBuffer::Create(const GLTrace* _trace) const
{
	CHECK_GL_ERROR();

	GLuint returnHandle = 0;
	::glGenBuffers(1, &returnHandle);
	CHECK_GL_ERROR();

	::glBindBuffer(mTarget, returnHandle);
	CHECK_GL_ERROR();

	::glBufferData(mTarget, mBufferSize, mBufferContents, mUsage);
	CHECK_GL_ERROR();

	return returnHandle;
}
