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
#include "gltexture.h"

#include "extensions.h"
#include "functionhooks.gen.h"

// ------------------------------------------------------------------------------------------------
static void DummyFunction()
{
	// If this fails, need to add a platform-specific GL_INT_MAX
	CompileTimeAssert(sizeof(int) == sizeof(GLint));
	#define GL_INT_MAX INT_MAX
}

// ------------------------------------------------------------------------------------------------
class GLError : std::exception
{
public:
	GLError(GLenum _err) 
	: mError(_err)
	{ }

	virtual const char* what() const { return "GLError"; }
private:
	GLenum mError;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLPixelStoreState::GLPixelStoreState()
: mData_GL_PACK_SWAP_BYTES(false)
, mData_GL_PACK_LSB_FIRST(false)
, mData_GL_PACK_ROW_LENGTH(0)
, mData_GL_PACK_IMAGE_HEIGHT(0)
, mData_GL_PACK_SKIP_PIXELS(0)
, mData_GL_PACK_SKIP_ROWS(0)
, mData_GL_PACK_SKIP_IMAGES(0) 
, mData_GL_PACK_ALIGNMENT(4)
, mData_GL_UNPACK_SWAP_BYTES(false)
, mData_GL_UNPACK_LSB_FIRST(false)
, mData_GL_UNPACK_ROW_LENGTH(0)
, mData_GL_UNPACK_IMAGE_HEIGHT(0)
, mData_GL_UNPACK_SKIP_PIXELS(0) 
, mData_GL_UNPACK_SKIP_ROWS(0)
, mData_GL_UNPACK_SKIP_IMAGES(0) 
, mData_GL_UNPACK_ALIGNMENT(4)
{
	
}

// ------------------------------------------------------------------------------------------------
void GLPixelStoreState::Write(FileLike* _out) const
{
	_out->Write(mData_GL_PACK_SWAP_BYTES);
	_out->Write(mData_GL_PACK_LSB_FIRST);
	_out->Write(mData_GL_PACK_ROW_LENGTH);
	_out->Write(mData_GL_PACK_IMAGE_HEIGHT);
	_out->Write(mData_GL_PACK_SKIP_PIXELS);
	_out->Write(mData_GL_PACK_SKIP_ROWS);
	_out->Write(mData_GL_PACK_SKIP_IMAGES); 
	_out->Write(mData_GL_PACK_ALIGNMENT);
	_out->Write(mData_GL_UNPACK_SWAP_BYTES);
	_out->Write(mData_GL_UNPACK_LSB_FIRST);
	_out->Write(mData_GL_UNPACK_ROW_LENGTH);
	_out->Write(mData_GL_UNPACK_IMAGE_HEIGHT);
	_out->Write(mData_GL_UNPACK_SKIP_PIXELS);
	_out->Write(mData_GL_UNPACK_SKIP_ROWS);
	_out->Write(mData_GL_UNPACK_SKIP_IMAGES);
	_out->Write(mData_GL_UNPACK_ALIGNMENT);
}

// ------------------------------------------------------------------------------------------------
void GLPixelStoreState::Read(FileLike* _in)
{
	_in->Read(&mData_GL_PACK_SWAP_BYTES);
	_in->Read(&mData_GL_PACK_LSB_FIRST);
	_in->Read(&mData_GL_PACK_ROW_LENGTH);
	_in->Read(&mData_GL_PACK_IMAGE_HEIGHT);
	_in->Read(&mData_GL_PACK_SKIP_PIXELS);
	_in->Read(&mData_GL_PACK_SKIP_ROWS);
	_in->Read(&mData_GL_PACK_SKIP_IMAGES); 
	_in->Read(&mData_GL_PACK_ALIGNMENT);
	_in->Read(&mData_GL_UNPACK_SWAP_BYTES);
	_in->Read(&mData_GL_UNPACK_LSB_FIRST);
	_in->Read(&mData_GL_UNPACK_ROW_LENGTH);
	_in->Read(&mData_GL_UNPACK_IMAGE_HEIGHT);
	_in->Read(&mData_GL_UNPACK_SKIP_PIXELS);
	_in->Read(&mData_GL_UNPACK_SKIP_ROWS);
	_in->Read(&mData_GL_UNPACK_SKIP_IMAGES);
	_in->Read(&mData_GL_UNPACK_ALIGNMENT);
}

// ------------------------------------------------------------------------------------------------
void GLPixelStoreState::glPixelStoref(GLenum pname, GLfloat param)
{
	switch (pname)
	{
	case GL_PACK_SWAP_BYTES:
		mData_GL_PACK_SWAP_BYTES = (GLboolean)param;
		break;
	case GL_PACK_LSB_FIRST:
		mData_GL_PACK_LSB_FIRST = (GLboolean)param;
		break;
	case GL_PACK_ROW_LENGTH: 
		mData_GL_PACK_ROW_LENGTH = (GLint)param;
		break;
	case GL_PACK_IMAGE_HEIGHT:
		mData_GL_PACK_IMAGE_HEIGHT = (GLint)param;
		break;
	case GL_PACK_SKIP_PIXELS:
		mData_GL_PACK_SKIP_PIXELS = (GLint)param;
		break;
	case GL_PACK_SKIP_ROWS:
		mData_GL_PACK_SKIP_ROWS = (GLint)param;
		break;
	case GL_PACK_SKIP_IMAGES:
		mData_GL_PACK_SKIP_IMAGES = (GLint)param;
		break;
	case GL_PACK_ALIGNMENT:
		mData_GL_PACK_ALIGNMENT = (GLint)param;
		break;
	case GL_UNPACK_SWAP_BYTES:
		mData_GL_UNPACK_SWAP_BYTES = (GLboolean)param;
		break;
	case GL_UNPACK_LSB_FIRST:
		mData_GL_UNPACK_LSB_FIRST = (GLboolean)param;
		break;
	case GL_UNPACK_ROW_LENGTH:
		mData_GL_UNPACK_ROW_LENGTH = (GLint)param;
		break;
	case GL_UNPACK_IMAGE_HEIGHT:
		mData_GL_UNPACK_IMAGE_HEIGHT = (GLint)param;
		break;
	case GL_UNPACK_SKIP_PIXELS:
		mData_GL_UNPACK_SKIP_PIXELS = (GLint)param;
		break;
	case GL_UNPACK_SKIP_ROWS:
		mData_GL_UNPACK_SKIP_ROWS = (GLint)param;
		break;
	case GL_UNPACK_SKIP_IMAGES:
		mData_GL_UNPACK_SKIP_IMAGES = (GLint)param;
		break;
	case GL_UNPACK_ALIGNMENT:
		mData_GL_UNPACK_ALIGNMENT = (GLint)param;
		break;
	default:
		assert(0);
	};
}

// ------------------------------------------------------------------------------------------------
void GLPixelStoreState::glPixelStorei(GLenum pname, GLint param)
{
	switch (pname)
	{
	case GL_PACK_SWAP_BYTES:
		mData_GL_PACK_SWAP_BYTES = (GLboolean)param;
		break;
	case GL_PACK_LSB_FIRST:
		mData_GL_PACK_LSB_FIRST = (GLboolean)param;
		break;
	case GL_PACK_ROW_LENGTH: 
		mData_GL_PACK_ROW_LENGTH = (GLint)param;
		break;
	case GL_PACK_IMAGE_HEIGHT:
		mData_GL_PACK_IMAGE_HEIGHT = (GLint)param;
		break;
	case GL_PACK_SKIP_PIXELS:
		mData_GL_PACK_SKIP_PIXELS = (GLint)param;
		break;
	case GL_PACK_SKIP_ROWS:
		mData_GL_PACK_SKIP_ROWS = (GLint)param;
		break;
	case GL_PACK_SKIP_IMAGES:
		mData_GL_PACK_SKIP_IMAGES = (GLint)param;
		break;
	case GL_PACK_ALIGNMENT:
		mData_GL_PACK_ALIGNMENT = (GLint)param;
		break;
	case GL_UNPACK_SWAP_BYTES:
		mData_GL_UNPACK_SWAP_BYTES = (GLboolean)param;
		break;
	case GL_UNPACK_LSB_FIRST:
		mData_GL_UNPACK_LSB_FIRST = (GLboolean)param;
		break;
	case GL_UNPACK_ROW_LENGTH:
		mData_GL_UNPACK_ROW_LENGTH = (GLint)param;
		break;
	case GL_UNPACK_IMAGE_HEIGHT:
		mData_GL_UNPACK_IMAGE_HEIGHT = (GLint)param;
		break;
	case GL_UNPACK_SKIP_PIXELS:
		mData_GL_UNPACK_SKIP_PIXELS = (GLint)param;
		break;
	case GL_UNPACK_SKIP_ROWS:
		mData_GL_UNPACK_SKIP_ROWS = (GLint)param;
		break;
	case GL_UNPACK_SKIP_IMAGES:
		mData_GL_UNPACK_SKIP_IMAGES = (GLint)param;
		break;
	case GL_UNPACK_ALIGNMENT:
		mData_GL_UNPACK_ALIGNMENT = (GLint)param;
		break;
	default:
		assert(0);
	};
}

// ------------------------------------------------------------------------------------------------
void GLPixelStoreState::Set() const
{
	::glPixelStorei(GL_PACK_SWAP_BYTES, mData_GL_PACK_SWAP_BYTES);
	::glPixelStorei(GL_PACK_LSB_FIRST, mData_GL_PACK_LSB_FIRST);
	::glPixelStorei(GL_PACK_ROW_LENGTH, mData_GL_PACK_ROW_LENGTH);
	::glPixelStorei(GL_PACK_IMAGE_HEIGHT, mData_GL_PACK_IMAGE_HEIGHT);
	::glPixelStorei(GL_PACK_SKIP_PIXELS, mData_GL_PACK_SKIP_PIXELS);
	::glPixelStorei(GL_PACK_SKIP_ROWS, mData_GL_PACK_SKIP_ROWS);
	::glPixelStorei(GL_PACK_SKIP_IMAGES, mData_GL_PACK_SKIP_IMAGES);
	::glPixelStorei(GL_PACK_ALIGNMENT, mData_GL_PACK_ALIGNMENT);
	::glPixelStorei(GL_UNPACK_SWAP_BYTES, mData_GL_UNPACK_SWAP_BYTES);
	::glPixelStorei(GL_UNPACK_LSB_FIRST, mData_GL_UNPACK_LSB_FIRST);
	::glPixelStorei(GL_UNPACK_ROW_LENGTH, mData_GL_UNPACK_ROW_LENGTH);
	::glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, mData_GL_UNPACK_IMAGE_HEIGHT);
	::glPixelStorei(GL_UNPACK_SKIP_PIXELS, mData_GL_UNPACK_SKIP_PIXELS);
	::glPixelStorei(GL_UNPACK_SKIP_ROWS, mData_GL_UNPACK_SKIP_ROWS);
	::glPixelStorei(GL_UNPACK_SKIP_IMAGES, mData_GL_UNPACK_SKIP_IMAGES);
	::glPixelStorei(GL_UNPACK_ALIGNMENT, mData_GL_UNPACK_ALIGNMENT);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLPixelTransferState::GLPixelTransferState()
: mData_GL_MAP_COLOR(false)
, mData_GL_MAP_STENCIL(false)
, mData_GL_INDEX_SHIFT(0)
, mData_GL_INDEX_OFFSET(0)
, mData_GL_RED_SCALE(1.0f)
, mData_GL_GREEN_SCALE(1.0f)
, mData_GL_BLUE_SCALE(1.0f)
, mData_GL_ALPHA_SCALE(1.0f)
, mData_GL_DEPTH_SCALE(1.0f)
, mData_GL_RED_BIAS(0.0f)
, mData_GL_GREEN_BIAS(0.0f)
, mData_GL_BLUE_BIAS(0.0f)
, mData_GL_ALPHA_BIAS(0.0f)
, mData_GL_DEPTH_BIAS(0.0f)
{

}


// ------------------------------------------------------------------------------------------------
void GLPixelTransferState::Write(FileLike* _out) const
{
	_out->Write(mData_GL_MAP_COLOR);
	_out->Write(mData_GL_MAP_STENCIL);
	_out->Write(mData_GL_INDEX_SHIFT);
	_out->Write(mData_GL_INDEX_OFFSET);
	_out->Write(mData_GL_RED_SCALE);
	_out->Write(mData_GL_GREEN_SCALE);
	_out->Write(mData_GL_BLUE_SCALE);
	_out->Write(mData_GL_ALPHA_SCALE);
	_out->Write(mData_GL_DEPTH_SCALE);
	_out->Write(mData_GL_RED_BIAS);
	_out->Write(mData_GL_GREEN_BIAS);
	_out->Write(mData_GL_BLUE_BIAS);
	_out->Write(mData_GL_ALPHA_BIAS);
	_out->Write(mData_GL_DEPTH_BIAS);
}

// ------------------------------------------------------------------------------------------------
void GLPixelTransferState::Read(FileLike* _in)
{
	_in->Read(&mData_GL_MAP_COLOR);
	_in->Read(&mData_GL_MAP_STENCIL);
	_in->Read(&mData_GL_INDEX_SHIFT);
	_in->Read(&mData_GL_INDEX_OFFSET);
	_in->Read(&mData_GL_RED_SCALE);
	_in->Read(&mData_GL_GREEN_SCALE);
	_in->Read(&mData_GL_BLUE_SCALE);
	_in->Read(&mData_GL_ALPHA_SCALE);
	_in->Read(&mData_GL_DEPTH_SCALE);
	_in->Read(&mData_GL_RED_BIAS);
	_in->Read(&mData_GL_GREEN_BIAS);
	_in->Read(&mData_GL_BLUE_BIAS);
	_in->Read(&mData_GL_ALPHA_BIAS);
	_in->Read(&mData_GL_DEPTH_BIAS);
}

// ------------------------------------------------------------------------------------------------
void GLPixelTransferState::Set() const
{
	::glPixelTransferi(GL_MAP_COLOR, mData_GL_MAP_COLOR);
	::glPixelTransferi(GL_MAP_STENCIL, mData_GL_MAP_STENCIL);
	::glPixelTransferi(GL_INDEX_SHIFT, mData_GL_INDEX_SHIFT);
	::glPixelTransferi(GL_INDEX_OFFSET, mData_GL_INDEX_OFFSET);
	::glPixelTransferf(GL_RED_SCALE, mData_GL_RED_SCALE);
	::glPixelTransferf(GL_GREEN_SCALE, mData_GL_GREEN_SCALE);
	::glPixelTransferf(GL_BLUE_SCALE, mData_GL_BLUE_SCALE);
	::glPixelTransferf(GL_ALPHA_SCALE, mData_GL_ALPHA_SCALE);
	::glPixelTransferf(GL_DEPTH_SCALE, mData_GL_DEPTH_SCALE);
	::glPixelTransferf(GL_RED_BIAS, mData_GL_RED_BIAS);
	::glPixelTransferf(GL_GREEN_BIAS, mData_GL_GREEN_BIAS);
	::glPixelTransferf(GL_BLUE_BIAS, mData_GL_BLUE_BIAS);
	::glPixelTransferf(GL_ALPHA_BIAS, mData_GL_ALPHA_BIAS);
	::glPixelTransferf(GL_DEPTH_BIAS, mData_GL_DEPTH_BIAS);

}

// ------------------------------------------------------------------------------------------------
void GLPixelTransferState::glPixelTransferf(GLenum pname, GLfloat param)
{
	switch (pname)
	{
	case GL_MAP_COLOR:
		mData_GL_MAP_COLOR = (GLboolean) param;
		break;
	case GL_MAP_STENCIL:
		mData_GL_MAP_STENCIL = (GLboolean) param;
		break;
	case GL_INDEX_SHIFT:
		mData_GL_INDEX_SHIFT = (GLint) param;
		break;
	case GL_INDEX_OFFSET:
		mData_GL_INDEX_OFFSET = (GLint) param;
		break;
	case GL_RED_SCALE:
		mData_GL_RED_SCALE = (GLfloat) param;
		break;
	case GL_GREEN_SCALE:
		mData_GL_GREEN_SCALE = (GLfloat) param;
		break;
	case GL_BLUE_SCALE:
		mData_GL_BLUE_SCALE = (GLfloat) param;
		break;
	case GL_ALPHA_SCALE:
		mData_GL_ALPHA_SCALE = (GLfloat) param;
		break;
	case GL_DEPTH_SCALE:
		mData_GL_DEPTH_SCALE = (GLfloat) param;
		break;
	case GL_RED_BIAS:
		mData_GL_RED_BIAS = (GLfloat) param;
		break;
	case GL_GREEN_BIAS:
		mData_GL_GREEN_BIAS = (GLfloat) param;
		break;
	case GL_BLUE_BIAS:
		mData_GL_BLUE_BIAS = (GLfloat) param;
		break;
	case GL_ALPHA_BIAS:
		mData_GL_ALPHA_BIAS = (GLfloat) param;
		break;
	case GL_DEPTH_BIAS:
		mData_GL_DEPTH_BIAS = (GLfloat) param;
		break;
	default:
		assert(0);
	};
}

// ------------------------------------------------------------------------------------------------
void GLPixelTransferState::glPixelTransferi(GLenum pname, GLint param)
{
	switch (pname)
	{
	case GL_MAP_COLOR:
		mData_GL_MAP_COLOR = (GLboolean) param;
		break;
	case GL_MAP_STENCIL:
		mData_GL_MAP_STENCIL = (GLboolean) param;
		break;
	case GL_INDEX_SHIFT:
		mData_GL_INDEX_SHIFT = (GLint) param;
		break;
	case GL_INDEX_OFFSET:
		mData_GL_INDEX_OFFSET = (GLint) param;
		break;
	case GL_RED_SCALE:
		mData_GL_RED_SCALE = (GLfloat) param;
		break;
	case GL_GREEN_SCALE:
		mData_GL_GREEN_SCALE = (GLfloat) param;
		break;
	case GL_BLUE_SCALE:
		mData_GL_BLUE_SCALE = (GLfloat) param;
		break;
	case GL_ALPHA_SCALE:
		mData_GL_ALPHA_SCALE = (GLfloat) param;
		break;
	case GL_DEPTH_SCALE:
		mData_GL_DEPTH_SCALE = (GLfloat) param;
		break;
	case GL_RED_BIAS:
		mData_GL_RED_BIAS = (GLfloat) param;
		break;
	case GL_GREEN_BIAS:
		mData_GL_GREEN_BIAS = (GLfloat) param;
		break;
	case GL_BLUE_BIAS:
		mData_GL_BLUE_BIAS = (GLfloat) param;
		break;
	case GL_ALPHA_BIAS:
		mData_GL_ALPHA_BIAS = (GLfloat) param;
		break;
	case GL_DEPTH_BIAS:
		mData_GL_DEPTH_BIAS = (GLfloat) param;
		break;
	default:
		assert(0);
	};
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
TextureUpdateData::TextureUpdateData(GLenum _target, GLint _level, GLint _internalFormat, GLsizei _width, GLsizei _height, GLint _border, GLenum _format, GLenum _type, GLvoid* _pixelData, size_t _pixelDataByteLength, bool _compressed, bool _subImageUpdate, const GLPixelStoreState& _pixelStoreState, const GLPixelTransferState& _pixelTransferState, int _xoffset, GLint _yoffset, GLint _depth)
: mTarget(_target)
, mLevel(_level)
, mXOffset(_xoffset)
, mYOffset(_yoffset)
, mDepth(_depth)
, mInternalFormat(_internalFormat)
, mWidth(_width)
, mHeight(_height)
, mBorder(_border)
, mFormat(_format)
, mType(_type)
, mPixelStoreState(_pixelStoreState)
, mPixelTransferState(_pixelTransferState)
, mPixelData(_pixelData)
, mPixelDataByteLength(_pixelDataByteLength)
, mCompressed(_compressed)
, mSubImageUpdate(_subImageUpdate)
{ 

}

// ------------------------------------------------------------------------------------------------
TextureUpdateData::TextureUpdateData(const TextureUpdateData& _rhs)
: mTarget(GL_NONE)
, mLevel(0)
, mXOffset(0)
, mYOffset(0)
, mDepth(-1)
, mInternalFormat(0)
, mWidth(0)
, mHeight(0)
, mBorder(0)
, mFormat(0)
, mType(0)
, mPixelData(NULL)
, mPixelDataByteLength(0)
, mCompressed(false)
, mSubImageUpdate(false)
{
	*this = _rhs;
}

// ------------------------------------------------------------------------------------------------
TextureUpdateData& TextureUpdateData::operator=(const TextureUpdateData& _rhs)
{
	if (this != &_rhs) {
		mTarget = _rhs.mTarget;
		mLevel = _rhs.mLevel;
		mXOffset = _rhs.mXOffset;
		mYOffset = _rhs.mYOffset;
		mDepth = _rhs.mDepth;
		mInternalFormat = _rhs.mInternalFormat;
		mWidth = _rhs.mWidth;
		mHeight = _rhs.mHeight;
		mBorder = _rhs.mBorder;
		mFormat = _rhs.mFormat;
		mType = _rhs.mType;
		mPixelStoreState = _rhs.mPixelStoreState;
		mPixelTransferState = _rhs.mPixelTransferState;

		if (_rhs.mPixelData) {
			mPixelDataByteLength = _rhs.mPixelDataByteLength;
			mPixelData = malloc(mPixelDataByteLength);
			memcpy(mPixelData, _rhs.mPixelData, mPixelDataByteLength);
		} 

		mCompressed = _rhs.mCompressed;
		mSubImageUpdate = _rhs.mSubImageUpdate;
	}

	return *this;
}

// ------------------------------------------------------------------------------------------------
TextureUpdateData& TextureUpdateData::operator=(TextureUpdateData&& _rhs)
{
	if (this != &_rhs) {
		mTarget = _rhs.mTarget;
		mLevel = _rhs.mLevel;
		mXOffset = _rhs.mXOffset;
		mYOffset = _rhs.mYOffset;
		mDepth = _rhs.mDepth;
		mInternalFormat = _rhs.mInternalFormat;
		mWidth = _rhs.mWidth;
		mHeight = _rhs.mHeight;
		mBorder = _rhs.mBorder;
		mFormat = _rhs.mFormat;
		mType = _rhs.mType;
		mPixelStoreState = _rhs.mPixelStoreState;
		mPixelTransferState = _rhs.mPixelTransferState;

		mPixelData = _rhs.mPixelData;
		mPixelDataByteLength = _rhs.mPixelDataByteLength;
		mCompressed = _rhs.mCompressed;
		mSubImageUpdate = _rhs.mSubImageUpdate;

		_rhs.mTarget = GL_NONE;
		_rhs.mLevel = 0;
		_rhs.mXOffset = 0;
		_rhs.mYOffset = 0;
		_rhs.mDepth = -1;
		_rhs.mInternalFormat = 0;
		_rhs.mWidth = 0;
		_rhs.mHeight = 0;
		_rhs.mBorder = 0;
		_rhs.mFormat = 0;
		_rhs.mType = 0;
		_rhs.mPixelData = NULL;
		_rhs.mPixelDataByteLength = 0;
		_rhs.mCompressed = false;
		_rhs.mSubImageUpdate = false;
	}

	return *this;
}

// ------------------------------------------------------------------------------------------------
void TextureUpdateData::Write(FileLike* _out) const
{
	_out->Write(mTarget);
	_out->Write(mLevel);
	_out->Write(mXOffset);
	_out->Write(mYOffset);
	_out->Write(mDepth);
	_out->Write(mInternalFormat);
	_out->Write(mWidth);
	_out->Write(mHeight);
	_out->Write(mBorder);
	_out->Write(mFormat);
	_out->Write(mType);
	_out->Write(mPixelStoreState);
	_out->Write(mPixelTransferState);
	// TODO: Check mUsedDuringCapture here
	_out->Write(mPixelData, mPixelDataByteLength);
	_out->Write(mCompressed);
	_out->Write(mSubImageUpdate);
}

// ------------------------------------------------------------------------------------------------
void TextureUpdateData::Read(FileLike* _in)
{
	_in->Read(&mTarget);
	_in->Read(&mLevel);
	_in->Read(&mXOffset);
	_in->Read(&mYOffset);
	_in->Read(&mDepth);
	_in->Read(&mInternalFormat);
	_in->Read(&mWidth);
	_in->Read(&mHeight);
	_in->Read(&mBorder);
	_in->Read(&mFormat);
	_in->Read(&mType);
	_in->Read(&mPixelStoreState);
	_in->Read(&mPixelTransferState);
	_in->Read(&mPixelData, &mPixelDataByteLength); 
	_in->Read(&mCompressed);
	_in->Read(&mSubImageUpdate);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLTexture::GLTexture(GLenum _target)
: mTarget(_target)
, mData_GL_TEXTURE_MIN_FILTER(GL_NEAREST_MIPMAP_LINEAR)
, mData_GL_TEXTURE_MAG_FILTER(GL_LINEAR)
, mData_GL_TEXTURE_MIN_LOD(-1000)
, mData_GL_TEXTURE_MAX_LOD(1000)
, mData_GL_TEXTURE_BASE_LEVEL(0)
, mData_GL_TEXTURE_MAX_LEVEL(1000)
, mData_GL_TEXTURE_WRAP_S(GL_REPEAT)
, mData_GL_TEXTURE_WRAP_T(GL_REPEAT)
, mData_GL_TEXTURE_WRAP_R(GL_REPEAT)
, mData_GL_TEXTURE_PRIORITY(1)
, mData_GL_TEXTURE_COMPARE_MODE(GL_NONE)
, mData_GL_TEXTURE_COMPARE_FUNC(GL_ALWAYS)
, mData_GL_TEXTURE_SRGB_DECODE_EXT(GL_DECODE_EXT)
, mData_GL_DEPTH_TEXTURE_MODE(GL_LUMINANCE)
, mData_GL_GENERATE_MIPMAP(GL_FALSE)
, mData_GL_TEXTURE_MAX_ANISOTROPY_EXT(1.0f)
{
	mData_GL_TEXTURE_BORDER_COLOR[0] = mData_GL_TEXTURE_BORDER_COLOR[1] = mData_GL_TEXTURE_BORDER_COLOR[2] = mData_GL_TEXTURE_BORDER_COLOR[3] = 0.0f;
}

// ------------------------------------------------------------------------------------------------
GLTexture::~GLTexture()
{

}

// ------------------------------------------------------------------------------------------------
void GLTexture::Write(FileLike* _out) const
{
	_out->Write(mTarget);

	_out->Write(mData_GL_TEXTURE_MIN_FILTER);
    _out->Write(mData_GL_TEXTURE_MAG_FILTER);
    _out->Write(mData_GL_TEXTURE_MIN_LOD);
    _out->Write(mData_GL_TEXTURE_MAX_LOD);
    _out->Write(mData_GL_TEXTURE_BASE_LEVEL);
    _out->Write(mData_GL_TEXTURE_MAX_LEVEL);
    _out->Write(mData_GL_TEXTURE_WRAP_S);
    _out->Write(mData_GL_TEXTURE_WRAP_T);
    _out->Write(mData_GL_TEXTURE_WRAP_R);
    _out->Write(mData_GL_TEXTURE_BORDER_COLOR[0]);
    _out->Write(mData_GL_TEXTURE_BORDER_COLOR[1]);
    _out->Write(mData_GL_TEXTURE_BORDER_COLOR[2]);
    _out->Write(mData_GL_TEXTURE_BORDER_COLOR[3]);
    _out->Write(mData_GL_TEXTURE_PRIORITY);
    _out->Write(mData_GL_TEXTURE_COMPARE_MODE);
    _out->Write(mData_GL_TEXTURE_COMPARE_FUNC);
    _out->Write(mData_GL_TEXTURE_SRGB_DECODE_EXT);
    _out->Write(mData_GL_DEPTH_TEXTURE_MODE);
    _out->Write(mData_GL_GENERATE_MIPMAP);

	_out->Write(mData_GL_TEXTURE_MAX_ANISOTROPY_EXT);

	_out->Write(mSequentialUpdates);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::Read(FileLike* _in)
{
	_in->Read(&mTarget);

	_in->Read(&mData_GL_TEXTURE_MIN_FILTER);
    _in->Read(&mData_GL_TEXTURE_MAG_FILTER);
    _in->Read(&mData_GL_TEXTURE_MIN_LOD);
    _in->Read(&mData_GL_TEXTURE_MAX_LOD);
    _in->Read(&mData_GL_TEXTURE_BASE_LEVEL);
    _in->Read(&mData_GL_TEXTURE_MAX_LEVEL);
    _in->Read(&mData_GL_TEXTURE_WRAP_S);
    _in->Read(&mData_GL_TEXTURE_WRAP_T);
    _in->Read(&mData_GL_TEXTURE_WRAP_R);
    _in->Read(&mData_GL_TEXTURE_BORDER_COLOR[0]);
    _in->Read(&mData_GL_TEXTURE_BORDER_COLOR[1]);
    _in->Read(&mData_GL_TEXTURE_BORDER_COLOR[2]);
    _in->Read(&mData_GL_TEXTURE_BORDER_COLOR[3]);
    _in->Read(&mData_GL_TEXTURE_PRIORITY);
    _in->Read(&mData_GL_TEXTURE_COMPARE_MODE);
    _in->Read(&mData_GL_TEXTURE_COMPARE_FUNC);
    _in->Read(&mData_GL_TEXTURE_SRGB_DECODE_EXT);
    _in->Read(&mData_GL_DEPTH_TEXTURE_MODE);
    _in->Read(&mData_GL_GENERATE_MIPMAP);

	_in->Read(&mData_GL_TEXTURE_MAX_ANISOTROPY_EXT);

	_in->Read(&mSequentialUpdates);

}

// ------------------------------------------------------------------------------------------------
void GLTexture::glBindTexture(GLenum target, GLuint texture)
{
	if (mTarget == GL_NONE) {
		mTarget = target;
	}

	assert(mTarget == target);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	if (mTarget == GL_NONE) {
		mTarget = target;
	}

	assert(mTarget == target);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glCompressedTexImage2D(ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data)
{
	// Checks for 
	if (!IsValid2DTarget(target)) {
		return;
	}

	if (IsCubemapTarget(target) && width != height) {
		return;
	}
	
	if (level < 0) { 
		return;
	}

	assert(mTarget == TexImage2DTargetToBoundTarget(target));
	
	GLvoid* ourCopy = NULL;
	size_t bufferLength = 0;
	if (data) {
		bufferLength = determinePointerLength_glCompressedTexImage2D_data(_ctxState, target, level, internalformat, width, height, border, imagesize, data);
		if (bufferLength != 0) {
			ourCopy = malloc(bufferLength);
			if (ourCopy) {
				memcpy(ourCopy, data, bufferLength);
			} else {
				assert(ourCopy && "glTexImage2D memory creation failed.");
				TraceError(TC("Attempting to create texture buffer of length %d failed in '%s'."), bufferLength, "glCompressedTexImage2D");
			}
		} else {
			assert(!"Couldn't determine pointer size from args, which is hilarious because it was given to us.");
			TraceError(TC("This bug will almost certainly be embarassing."));
		}
	}

	TextureUpdateData update(target, level, internalformat, width, height, border, 0, 0, ourCopy, bufferLength, true, false, _ctxState->GetPixelStoreState(), _ctxState->GetPixelTransferState());
	mSequentialUpdates = AppendTextureUpdate(mSequentialUpdates, update);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glCompressedTexImage3D(ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid* data)
{
	assert(0);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexImage2D(ContextState* _ctxState, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	if (!IsValid2DTarget(target)) {
		return;
	}

	if (IsCubemapTarget(target) && width != height) {
		return;
	}
	
	if (level < 0) { 
		return;
	}

	// TODO: Check for other bogus stuff here
	assert(mTarget == TexImage2DTargetToBoundTarget(target));

	// If pixels is non-null, we need to copy data out into our own buffer so we can send it across the wire.
	GLvoid* ourCopy = NULL;
	size_t bufferLength = 0;
	if (pixels) {
		bufferLength = determinePointerLength_glTexImage2D_pixels(_ctxState, target, level, internalformat, width, height, border, format, type, pixels);
		// TODO: If bufferLength is zero but pixels isn't, we should inject a surface in so that hopefully the trace can play with *some* data.
		// But for now, if it's NULL, just leave it NULL and at least the surface will be created on our behalf.
		if (bufferLength != 0) {
			ourCopy = malloc(bufferLength);
			if (ourCopy) {
				memcpy(ourCopy, pixels, bufferLength);
			} else {
				assert(ourCopy && "glTexImage2D memory creation failed.");
				TraceError(TC("Attempting to create texture buffer of length %d failed in '%s'."), bufferLength, "glTexImage2D");
			}
		} else {
			assert(!"Couldn't determine pointer size from args.");
			TraceError(TC("glTexImage2D was given args that my primitive man-brain couldn't understand. A debugger will need to be attached to find the right buffer size."));
		}
	}

	TextureUpdateData update(target, level, internalformat, width, height, border, format, type, ourCopy, bufferLength, false, false, _ctxState->GetPixelStoreState(), _ctxState->GetPixelTransferState());
	mSequentialUpdates = AppendTextureUpdate(mSequentialUpdates, update);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexImage3D(ContextState* _ctxState, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
	if (!IsValid3DTarget(target)) {
		return;
	}
	
	if (level < 0) { 
		return;
	}

	// I use this as a sentinel, so if this hits bad stuff will happen (and we'll need to do a separate boolean for is3D).
	assert(depth != -1);
		
	// TODO: Check for other bogus stuff here
	assert(mTarget == target);

	// If pixels is non-null, we need to copy data out into our own buffer so we can send it across the wire.
	GLvoid* ourCopy = NULL;
	size_t bufferLength = 0;
	if (data) {
		bufferLength = determinePointerLength_glTexImage3D_data(_ctxState, target, level, internalFormat, width, height, depth, border, format, type, data);
		// TODO: If bufferLength is zero but pixels isn't, we should inject a surface in so that hopefully the trace can play with *some* data.
		// But for now, if it's NULL, just leave it NULL and at least the surface will be created on our behalf.
		if (bufferLength != 0) {
			ourCopy = malloc(bufferLength);
			if (ourCopy) {
				memcpy(ourCopy, data, bufferLength);
			} else {
				assert(ourCopy && "glTexImage3D memory creation failed.");
				TraceError(TC("Attempting to create texture buffer of length %d failed in '%s'."), bufferLength, "glTexImage3D");
			}
		} else {
			assert(!"Couldn't determine pointer size from args.");
			TraceError(TC("glTexImage3D was given args that my primitive man-brain couldn't understand. A debugger will need to be attached to find the right buffer size."));
		}
	}

	TextureUpdateData update(target, level, internalFormat, width, height, border, format, type, ourCopy, bufferLength, false, false, _ctxState->GetPixelStoreState(), _ctxState->GetPixelTransferState(), 0, 0, depth);
	mSequentialUpdates = AppendTextureUpdate(mSequentialUpdates, update);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexSubImage2D(ContextState* _ctxState, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	if (!IsValid2DTarget(target)) {
		return;
	}

	if (IsCubemapTarget(target) && width != height) {
		return;
	}
	
	if (level < 0) { 
		return;
	}

	// TODO: Check for other bogus stuff here
	assert(mTarget == TexImage2DTargetToBoundTarget(target));

	// If pixels is non-null, we need to copy data out into our own buffer so we can send it across the wire.
	GLvoid* ourCopy = NULL;
	size_t bufferLength = 0;
	if (pixels) {
		bufferLength = determinePointerLength_glTexSubImage2D_pixels(_ctxState, target, level, xoffset, yoffset, width, height, format, type, pixels);
		// TODO: If bufferLength is zero but pixels isn't, we should inject a surface in so that hopefully the trace can play with *some* data.
		// But for now, if it's NULL, just leave it NULL and at least the surface will be created on our behalf.
		if (bufferLength != 0) {
			ourCopy = malloc(bufferLength);
			if (ourCopy) {
				memcpy(ourCopy, pixels, bufferLength);
			} else {
				assert(ourCopy && "glTexImage2D memory creation failed.");
				TraceError(TC("Attempting to create texture buffer of length %d failed in '%s'."), bufferLength, "glTexSubImage2D");
			}
		} else {
			assert(!"Couldn't determine pointer size from args.");
			TraceError(TC("glTexSubImage2D was given args that my primitive man-brain couldn't understand. A debugger will need to be attached to find the right buffer size."));
		}
	}

	TextureUpdateData update(target, level, 0, width, height, 0, format, type, ourCopy, bufferLength, false, true, _ctxState->GetPixelStoreState(), _ctxState->GetPixelTransferState(), xoffset, yoffset);
	mSequentialUpdates = AppendTextureUpdate(mSequentialUpdates, update);
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexParameterf(GLenum pname, GLfloat param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_BASE_LEVEL:
		mData_GL_TEXTURE_BASE_LEVEL = (GLint)param;
		break;
    case GL_TEXTURE_MAX_LEVEL:
		mData_GL_TEXTURE_MAX_LEVEL = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)param;
		break;
    case GL_TEXTURE_PRIORITY:
		mData_GL_TEXTURE_PRIORITY = (GLfloat)param;
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)param;
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint)param;
        break;
    case GL_DEPTH_TEXTURE_MODE:
		mData_GL_DEPTH_TEXTURE_MODE = (GLint)param;
		break;
    case GL_GENERATE_MIPMAP:
		mData_GL_GENERATE_MIPMAP = (GLint)param;
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)param;
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexParameteri(GLenum pname, GLint param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_BASE_LEVEL:
		mData_GL_TEXTURE_BASE_LEVEL = (GLint)param;
		break;
    case GL_TEXTURE_MAX_LEVEL:
		mData_GL_TEXTURE_MAX_LEVEL = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)param;
		break;
    case GL_TEXTURE_PRIORITY:
		mData_GL_TEXTURE_PRIORITY = (GLfloat)param;
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)param;
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint)param;
        break;
    case GL_DEPTH_TEXTURE_MODE:
		mData_GL_DEPTH_TEXTURE_MODE = (GLint)param;
		break;
    case GL_GENERATE_MIPMAP:
		mData_GL_GENERATE_MIPMAP = (GLint)param;
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)param;
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexParameterfv(GLenum pname, const GLfloat *param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)(*param);
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)(*param);
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)(*param);
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)(*param);
		break;
    case GL_TEXTURE_BASE_LEVEL:
		mData_GL_TEXTURE_BASE_LEVEL = (GLint)(*param);
		break;
    case GL_TEXTURE_MAX_LEVEL:
		mData_GL_TEXTURE_MAX_LEVEL = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)(*param);
		break;
    case GL_TEXTURE_BORDER_COLOR:
		mData_GL_TEXTURE_BORDER_COLOR[0] = param[0];
		mData_GL_TEXTURE_BORDER_COLOR[1] = param[1];
		mData_GL_TEXTURE_BORDER_COLOR[2] = param[2];
		mData_GL_TEXTURE_BORDER_COLOR[3] = param[3];
		break;
    case GL_TEXTURE_PRIORITY:
		mData_GL_TEXTURE_PRIORITY = (GLfloat)(*param);
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)(*param);
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)(*param);
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint)param;
        break;
    case GL_DEPTH_TEXTURE_MODE:
		mData_GL_DEPTH_TEXTURE_MODE = (GLint)(*param);
		break;
    case GL_GENERATE_MIPMAP:
		mData_GL_GENERATE_MIPMAP = (GLint)(*param);
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)(*param);
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLTexture::glTexParameteriv(GLenum pname, const GLint *param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)(*param);
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)(*param);
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)(*param);
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)(*param);
		break;
    case GL_TEXTURE_BASE_LEVEL:
		mData_GL_TEXTURE_BASE_LEVEL = (GLint)(*param);
		break;
    case GL_TEXTURE_MAX_LEVEL:
		mData_GL_TEXTURE_MAX_LEVEL = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)(*param);
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)(*param);
		break;
    case GL_TEXTURE_BORDER_COLOR:
		mData_GL_TEXTURE_BORDER_COLOR[0] = 1.0f * param[0] / GL_INT_MAX;
		mData_GL_TEXTURE_BORDER_COLOR[1] = 1.0f * param[1] / GL_INT_MAX;
		mData_GL_TEXTURE_BORDER_COLOR[2] = 1.0f * param[2] / GL_INT_MAX;
		mData_GL_TEXTURE_BORDER_COLOR[3] = 1.0f * param[3] / GL_INT_MAX;
		break;
    case GL_TEXTURE_PRIORITY:
		mData_GL_TEXTURE_PRIORITY = (GLfloat)(*param);
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)(*param);
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)(*param);
		break;
    case GL_DEPTH_TEXTURE_MODE:
		mData_GL_DEPTH_TEXTURE_MODE = (GLint)(*param);
		break;
    case GL_GENERATE_MIPMAP:
		mData_GL_GENERATE_MIPMAP = (GLint)(*param);
		break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)(*param);
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
GLuint GLTexture::Create(const GLTrace* _glTrace) const
{
	CHECK_GL_ERROR();

	GLuint returnHandle = 0;
	::glGenTextures(1, &returnHandle);
	CHECK_GL_ERROR();

	if (mTarget) {
		::glBindTexture(mTarget, returnHandle);
		CHECK_GL_ERROR();

		for (auto it = mSequentialUpdates.cbegin(); it != mSequentialUpdates.cend(); ++it) {
			it->mPixelStoreState.Set();
			CHECK_GL_ERROR();
			it->mPixelTransferState.Set();
			CHECK_GL_ERROR();
			if (it->IsSubImageUpdate()) {
				if (it->Is2D()) {
					if (it->IsCompressed()) {
						assert(0);
					} else {
						::glTexSubImage2D(it->mTarget, it->mLevel, it->mXOffset, it->mYOffset, it->mWidth, it->mHeight, it->mFormat, it->mType, it->GetPixelData());
					}
				} else {
					// Don't deal with 1D or 3D SubImage updates atm.
					assert(0);
				}
				CHECK_GL_ERROR();
			} else {
				if (it->Is2D()) {
					if (it->IsCompressed()) {
						::glCompressedTexImage2D(it->mTarget, it->mLevel, it->mInternalFormat, it->mWidth, it->mHeight, it->mBorder, it->GetPixelDataByteLength(), it->GetPixelData());
					} else {
						::glTexImage2D(it->mTarget, it->mLevel, it->mInternalFormat, it->mWidth, it->mHeight, it->mBorder, it->mFormat, it->mType, it->GetPixelData());
					}
				} else if (it->Is3D()) {
					::glTexImage3D(it->mTarget, it->mLevel, it->mInternalFormat, it->mWidth, it->mHeight, it->mDepth, it->mBorder, it->mFormat, it->mType, it->GetPixelData());

				} else {
					// Shouldn't happen right now--I don't do 1D textures yet.
					assert(0);
				}
				CHECK_GL_ERROR();
			}

		}
	} else {
		assert(mSequentialUpdates.size() == 0);
	}

	// TODO: Set texture state. Doh.

	return returnHandle;
}

// ------------------------------------------------------------------------------------------------
std::vector<TextureUpdateData> GLTexture::AppendTextureUpdate(const std::vector<TextureUpdateData>& _currentList, const TextureUpdateData& _update)
{
	std::vector<TextureUpdateData> retVal;
	retVal.reserve(_currentList.size() + 1);
	
	Rect2D updateRect = _update.GetUpdateRect();
	for (auto it = _currentList.begin(); it != _currentList.end(); ++it) {
		// Filter out updates to the same target and level, because this supercedes those.
		// skipping the push at the bottom of the loop will filter out a previous update.
		auto levelTargetPair = std::make_pair(it->mLevel, it->mTarget);

		if (it->mLevel == _update.mLevel && it->mTarget == _update.mTarget) {
			if (_update.mSubImageUpdate == false) {
				continue;
			}

#if 0 // This needs a little more love to make it robust
			// Need to keep the first update because we need a texImage2D to setup the surface.
			// We could destroy the texels that it stores, but don't for now--I'm lazy.
			if (updateRect.Contains(it->GetUpdateRect())) {
				continue;
			}
#endif
		}

		retVal.push_back(*it);
	}

	retVal.push_back(_update);
	return retVal;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GLSampler::GLSampler(GLuint sampler)
: mSampler(sampler)
, mData_GL_TEXTURE_MIN_FILTER(GL_NEAREST_MIPMAP_LINEAR)
, mData_GL_TEXTURE_MAG_FILTER(GL_LINEAR)
, mData_GL_TEXTURE_MIN_LOD(-1000)
, mData_GL_TEXTURE_MAX_LOD(1000)
, mData_GL_TEXTURE_WRAP_S(GL_REPEAT)
, mData_GL_TEXTURE_WRAP_T(GL_REPEAT)
, mData_GL_TEXTURE_WRAP_R(GL_REPEAT)
, mData_GL_TEXTURE_COMPARE_MODE(GL_NONE)
, mData_GL_TEXTURE_COMPARE_FUNC(GL_ALWAYS)
, mData_GL_TEXTURE_SRGB_DECODE_EXT(GL_DECODE_EXT)
, mData_GL_TEXTURE_MAX_ANISOTROPY_EXT(1.0f)
{ 

}

// ------------------------------------------------------------------------------------------------
GLSampler::~GLSampler()
{

}

// ------------------------------------------------------------------------------------------------
void GLSampler::Write(FileLike* _out) const
{
	_out->Write(mSampler);
	_out->Write(mData_GL_TEXTURE_MIN_FILTER);
	_out->Write(mData_GL_TEXTURE_MAG_FILTER);
	_out->Write(mData_GL_TEXTURE_MIN_LOD);
	_out->Write(mData_GL_TEXTURE_MAX_LOD);
	_out->Write(mData_GL_TEXTURE_WRAP_S);
	_out->Write(mData_GL_TEXTURE_WRAP_T);
	_out->Write(mData_GL_TEXTURE_WRAP_R);
	_out->Write(mData_GL_TEXTURE_COMPARE_MODE);
	_out->Write(mData_GL_TEXTURE_COMPARE_FUNC);
    _out->Write(mData_GL_TEXTURE_SRGB_DECODE_EXT);
	_out->Write(mData_GL_TEXTURE_MAX_ANISOTROPY_EXT);
}

// ------------------------------------------------------------------------------------------------
void GLSampler::Read(FileLike* _in)
{
	_in->Read(&mSampler);
	_in->Read(&mData_GL_TEXTURE_MIN_FILTER);
	_in->Read(&mData_GL_TEXTURE_MAG_FILTER);
	_in->Read(&mData_GL_TEXTURE_MIN_LOD);
	_in->Read(&mData_GL_TEXTURE_MAX_LOD);
	_in->Read(&mData_GL_TEXTURE_WRAP_S);
	_in->Read(&mData_GL_TEXTURE_WRAP_T);
	_in->Read(&mData_GL_TEXTURE_WRAP_R);
	_in->Read(&mData_GL_TEXTURE_COMPARE_MODE);
	_in->Read(&mData_GL_TEXTURE_COMPARE_FUNC);
    _in->Read(&mData_GL_TEXTURE_SRGB_DECODE_EXT);
	_in->Read(&mData_GL_TEXTURE_MAX_ANISOTROPY_EXT);
}

// ------------------------------------------------------------------------------------------------
void GLSampler::glBindSampler(GLuint unit, GLuint sampler)
{
	if (mSampler == 0) {
		mSampler = sampler;
	}

	// To catch bugs.
	assert(mSampler == sampler);
}

// ------------------------------------------------------------------------------------------------
void GLSampler::glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)param;
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint) param;
        break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)param;
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLSampler::glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)param;
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)param;
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)param;
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)param;
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)param;
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint) param;
        break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)param;
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
void GLSampler::glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* params)
{
	switch(pname)
	{
    case GL_TEXTURE_MIN_FILTER:
		mData_GL_TEXTURE_MIN_FILTER = (GLint)(*params);
		break;
    case GL_TEXTURE_MAG_FILTER:
		mData_GL_TEXTURE_MAG_FILTER = (GLint)(*params);
		break;
    case GL_TEXTURE_MIN_LOD:
		mData_GL_TEXTURE_MIN_LOD = (GLfloat)(*params);
		break;
    case GL_TEXTURE_MAX_LOD:
		mData_GL_TEXTURE_MAX_LOD = (GLfloat)(*params);
		break;
    case GL_TEXTURE_WRAP_S:
		mData_GL_TEXTURE_WRAP_S = (GLint)(*params);
		break;
    case GL_TEXTURE_WRAP_T:
		mData_GL_TEXTURE_WRAP_T = (GLint)(*params);
		break;
    case GL_TEXTURE_WRAP_R:
		mData_GL_TEXTURE_WRAP_R = (GLint)(*params);
		break;
    case GL_TEXTURE_BORDER_COLOR:
		mData_GL_TEXTURE_BORDER_COLOR[0] = params[0];
		mData_GL_TEXTURE_BORDER_COLOR[1] = params[1];
		mData_GL_TEXTURE_BORDER_COLOR[2] = params[2];
		mData_GL_TEXTURE_BORDER_COLOR[3] = params[3];
		break;
    case GL_TEXTURE_COMPARE_MODE:
		mData_GL_TEXTURE_COMPARE_MODE = (GLint)(*params);
		break;
    case GL_TEXTURE_COMPARE_FUNC:
		mData_GL_TEXTURE_COMPARE_FUNC = (GLint)(*params);
		break;
    case GL_TEXTURE_SRGB_DECODE_EXT:
        mData_GL_TEXTURE_SRGB_DECODE_EXT = (GLint)(*params);
        break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT:
		mData_GL_TEXTURE_MAX_ANISOTROPY_EXT = (GLfloat)(*params);
		break;
	default:
		assert(0);
	}
}

// ------------------------------------------------------------------------------------------------
GLuint GLSampler::Create(const GLTrace* _trace) const
{
	GLuint retVal = 0;
	::glGenSamplers(1, &retVal);
	CHECK_GL_ERROR();

	::glSamplerParameteri (retVal, GL_TEXTURE_MIN_FILTER, mData_GL_TEXTURE_MIN_FILTER);
	::glSamplerParameteri (retVal, GL_TEXTURE_MAG_FILTER, mData_GL_TEXTURE_MAG_FILTER);
	::glSamplerParameterf (retVal, GL_TEXTURE_MIN_LOD, mData_GL_TEXTURE_MIN_LOD);
	::glSamplerParameterf (retVal, GL_TEXTURE_MAX_LOD, mData_GL_TEXTURE_MAX_LOD);
	::glSamplerParameteri (retVal, GL_TEXTURE_WRAP_S, mData_GL_TEXTURE_WRAP_S);
	::glSamplerParameteri (retVal, GL_TEXTURE_WRAP_T, mData_GL_TEXTURE_WRAP_T);
	::glSamplerParameteri (retVal, GL_TEXTURE_WRAP_R, mData_GL_TEXTURE_WRAP_R);
	::glSamplerParameterfv(retVal, GL_TEXTURE_BORDER_COLOR, mData_GL_TEXTURE_BORDER_COLOR);
	::glSamplerParameteri (retVal, GL_TEXTURE_COMPARE_MODE, mData_GL_TEXTURE_COMPARE_MODE);
	::glSamplerParameteri (retVal, GL_TEXTURE_COMPARE_FUNC, mData_GL_TEXTURE_COMPARE_FUNC);
    ::glSamplerParameteri (retVal, GL_TEXTURE_SRGB_DECODE_EXT, mData_GL_TEXTURE_SRGB_DECODE_EXT);
	::glSamplerParameterf (retVal, GL_TEXTURE_MAX_ANISOTROPY_EXT, mData_GL_TEXTURE_MAX_ANISOTROPY_EXT);
	CHECK_GL_ERROR();

	return retVal;
}
