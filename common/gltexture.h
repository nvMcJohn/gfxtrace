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

#include <vector>

class ContextState;
class GLTrace;

struct Rect2D
{
	int X0, Y0;
	int X1, Y1;

	Rect2D(int _x0 = 0, int _y0 = 0, int _x1 = 0, int _y1 = 0) 
	: X0(_x0)
	, Y0(_y0)
	, X1(_x1)
	, Y1(_y1)
	{ }

	bool Contains(const Rect2D& _rhs) const
	{
		return X0 <= _rhs.X0
			&& Y0 <= _rhs.Y0
			&& X1 >= _rhs.X1
			&& Y1 >= _rhs.Y1;
	}

};

// ------------------------------------------------------------------------------------------------
inline bool IsCubemapTarget(GLenum _target)
{
	switch(_target) {
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X: 
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: 
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: 
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: 
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: 
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			return true;
		default:
			break;
	};
	return false;
}

// ------------------------------------------------------------------------------------------------
inline bool IsValid2DTarget(GLenum _target)
{
	if (IsCubemapTarget(_target)) {
		return true;
	}

	switch(_target) {
		case GL_TEXTURE_2D:
		case GL_PROXY_TEXTURE_2D:
		case GL_PROXY_TEXTURE_CUBE_MAP: 
			return true;
		default:
			break;
	};
	return false;
}

// ------------------------------------------------------------------------------------------------
inline bool IsValid3DTarget(GLenum _target)
{
	switch (_target) {
	case GL_TEXTURE_3D:
	case GL_PROXY_TEXTURE_3D:
		return true;
	default:
		break;
	};
	return false;
}

// ------------------------------------------------------------------------------------------------
class GLPixelStoreState
{
public:
	GLPixelStoreState();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glPixelStoref(GLenum pname, GLfloat param);
	void glPixelStorei(GLenum pname, GLint param);

	template <typename T>
	T glGet(GLenum pname, T* _out=NULL) const
	{
		switch (pname)
		{
		case GL_PACK_SWAP_BYTES: return (T)mData_GL_PACK_SWAP_BYTES;
		case GL_PACK_LSB_FIRST: return (T)mData_GL_PACK_LSB_FIRST;
		case GL_PACK_ROW_LENGTH: return (T)mData_GL_PACK_ROW_LENGTH;
		case GL_PACK_IMAGE_HEIGHT: return (T)mData_GL_PACK_IMAGE_HEIGHT;
		case GL_PACK_SKIP_PIXELS: return (T)mData_GL_PACK_SKIP_PIXELS;
		case GL_PACK_SKIP_ROWS: return (T)mData_GL_PACK_SKIP_ROWS;
		case GL_PACK_SKIP_IMAGES: return (T)mData_GL_PACK_SKIP_IMAGES;
		case GL_PACK_ALIGNMENT: return (T)mData_GL_PACK_ALIGNMENT;
		case GL_UNPACK_SWAP_BYTES: return (T)mData_GL_UNPACK_SWAP_BYTES;
		case GL_UNPACK_LSB_FIRST: return (T)mData_GL_UNPACK_LSB_FIRST;
		case GL_UNPACK_ROW_LENGTH: return (T)mData_GL_UNPACK_ROW_LENGTH;
		case GL_UNPACK_IMAGE_HEIGHT: return (T)mData_GL_UNPACK_IMAGE_HEIGHT;
		case GL_UNPACK_SKIP_PIXELS: return (T)mData_GL_UNPACK_SKIP_PIXELS;
		case GL_UNPACK_SKIP_ROWS: return (T)mData_GL_UNPACK_SKIP_ROWS;
		case GL_UNPACK_SKIP_IMAGES: return (T)mData_GL_UNPACK_SKIP_IMAGES;
		case GL_UNPACK_ALIGNMENT: return (T)mData_GL_UNPACK_ALIGNMENT;
		default:
			assert(0);
		};
		return T(0);
	}

	void Set() const;

private:
	// Packing
	GLboolean mData_GL_PACK_SWAP_BYTES; 
	GLboolean mData_GL_PACK_LSB_FIRST; 
	GLint mData_GL_PACK_ROW_LENGTH; 
	GLint mData_GL_PACK_IMAGE_HEIGHT; 
	GLint mData_GL_PACK_SKIP_PIXELS; 
	GLint mData_GL_PACK_SKIP_ROWS; 
	GLint mData_GL_PACK_SKIP_IMAGES; 
	GLint mData_GL_PACK_ALIGNMENT; 

	// Unpacking
	GLboolean mData_GL_UNPACK_SWAP_BYTES; 
	GLboolean mData_GL_UNPACK_LSB_FIRST; 
	GLint mData_GL_UNPACK_ROW_LENGTH; 
	GLint mData_GL_UNPACK_IMAGE_HEIGHT; 
	GLint mData_GL_UNPACK_SKIP_PIXELS; 
	GLint mData_GL_UNPACK_SKIP_ROWS; 
	GLint mData_GL_UNPACK_SKIP_IMAGES; 
	GLint mData_GL_UNPACK_ALIGNMENT;
};

// ------------------------------------------------------------------------------------------------
class GLPixelTransferState
{
public:
	GLPixelTransferState();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glPixelTransferf(GLenum pname, GLfloat param);
	void glPixelTransferi(GLenum pname, GLint param);

	template <typename T>
	T glGet(GLenum pname, T* _out=NULL) const
	{
		switch (pname)
		{
		case GL_MAP_COLOR: return (T)mData_GL_MAP_COLOR;
		case GL_MAP_STENCIL: return (T)mData_GL_MAP_STENCIL;
		case GL_INDEX_SHIFT: return (T)mData_GL_INDEX_SHIFT;
		case GL_INDEX_OFFSET: return (T)mData_GL_INDEX_OFFSET;
		case GL_RED_SCALE: return (T)mData_GL_RED_SCALE;
		case GL_GREEN_SCALE: return (T)mData_GL_GREEN_SCALE;
		case GL_BLUE_SCALE: return (T)mData_GL_BLUE_SCALE;
		case GL_ALPHA_SCALE: return (T)mData_GL_ALPHA_SCALE;
		case GL_DEPTH_SCALE: return (T)mData_GL_DEPTH_SCALE;
		case GL_RED_BIAS: return (T)mData_GL_RED_BIAS;
		case GL_GREEN_BIAS: return (T)mData_GL_GREEN_BIAS;
		case GL_BLUE_BIAS: return (T)mData_GL_BLUE_BIAS;
		case GL_ALPHA_BIAS: return (T)mData_GL_ALPHA_BIAS;
		case GL_DEPTH_BIAS: return (T)mData_GL_DEPTH_BIAS;
		default:
			assert(0);
		};
		return T(0);
	}

	void Set() const;

private:
	GLboolean mData_GL_MAP_COLOR;
	GLboolean mData_GL_MAP_STENCIL;
	GLint mData_GL_INDEX_SHIFT;
	GLint mData_GL_INDEX_OFFSET;
	GLfloat mData_GL_RED_SCALE;
	GLfloat mData_GL_GREEN_SCALE;
	GLfloat mData_GL_BLUE_SCALE;
	GLfloat mData_GL_ALPHA_SCALE;
	GLfloat mData_GL_DEPTH_SCALE;
	GLfloat mData_GL_RED_BIAS;
	GLfloat mData_GL_GREEN_BIAS;
	GLfloat mData_GL_BLUE_BIAS;
	GLfloat mData_GL_ALPHA_BIAS;
	GLfloat mData_GL_DEPTH_BIAS;
};

// ------------------------------------------------------------------------------------------------
class TextureUpdateData
{
public:
	// For all updates (compressed and uncompressed)
	TextureUpdateData(GLenum _target = 0, GLint _level = 0, GLint _internalFormat = 0, GLsizei _width = 0, GLsizei _height = 0, GLint _border = 0, GLenum _format = 0, GLenum _type = 0, GLvoid* _pixelData = NULL, size_t _pixelDataByteLength = 0, bool _compressed = false, bool _subImageUpdate = false,
			const GLPixelStoreState& _pixelStoreState = GLPixelStoreState(), const GLPixelTransferState& _pixelTransferState = GLPixelTransferState(), GLint _xoffset = 0, GLint _yoffset = 0, GLint _depth = -1);

	// Copy and Move-constructors
	TextureUpdateData(const TextureUpdateData& _rhs);
	TextureUpdateData(TextureUpdateData&& _rhs) { *this = std::move(_rhs); }
	~TextureUpdateData() { ReleaseData(); }

	// Assignment and Move-assignment
	TextureUpdateData& operator=(const TextureUpdateData& _rhs);
	TextureUpdateData& operator=(TextureUpdateData&& _rhs);

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	const GLvoid* GetPixelData() const { return mPixelData; }
	size_t GetPixelDataByteLength() const { return mPixelDataByteLength; }
	bool IsCompressed() const { return mCompressed; }
	bool IsSubImageUpdate() const { return mSubImageUpdate; }
	bool Is2D() const { return mDepth == -1; }
	bool Is3D() const { return mDepth != -1; }

	inline Rect2D GetUpdateRect() const
	{
		return Rect2D(mXOffset, 
                      mYOffset, 
					  mWidth  + mXOffset - 1, 
					  mHeight + mYOffset - 1);
	}

private:
	GLenum mTarget;

	GLint mLevel;
	GLint mInternalFormat;
	GLint mXOffset;
	GLint mYOffset;
	GLint mDepth;
	GLsizei mWidth;
	GLsizei mHeight;
	GLint mBorder; 
	GLenum mFormat;
	GLenum mType;

	GLPixelStoreState mPixelStoreState;
	GLPixelTransferState mPixelTransferState;

	GLvoid* mPixelData;
	size_t mPixelDataByteLength;

	bool mCompressed;
	bool mSubImageUpdate;

	inline void ReleaseData()
	{
		if (mPixelData) {
			free(mPixelData);
			mPixelData = NULL;
		}

		mPixelDataByteLength = 0;
	}

	// TODO: Add accessors, get rid of friendliness.
	friend class GLTexture;
};

// ------------------------------------------------------------------------------------------------
class GLTexture
{
public:
	GLTexture(GLenum _target=GL_NONE);
	~GLTexture();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glBindTexture(GLenum target, GLuint texture);
	void glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture);
	void glCompressedTexImage2D(ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data);
	void glCompressedTexImage3D(ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid* data);
	void glTexImage2D(ContextState* _ctxState, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
	void glTexImage3D(ContextState* _ctxState, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data);
	void glTexSubImage2D(ContextState* _ctxState, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);

	void glTexParameterf(GLenum pname, GLfloat param);
	void glTexParameteri(GLenum pname, GLint param);

	void glTexParameterfv(GLenum pname, const GLfloat *param);
	void glTexParameteriv(GLenum pname, const GLint *param);


	GLuint Create(const GLTrace* _trace) const;

private:
	// Stored both here and in the update to determine if we need to bail out early.
	GLenum mTarget;

	// Sampler data
	GLint mData_GL_TEXTURE_MIN_FILTER;
    GLint mData_GL_TEXTURE_MAG_FILTER;
    GLfloat mData_GL_TEXTURE_MIN_LOD;
    GLfloat mData_GL_TEXTURE_MAX_LOD;
    GLint mData_GL_TEXTURE_BASE_LEVEL;
    GLint mData_GL_TEXTURE_MAX_LEVEL;
    GLint mData_GL_TEXTURE_WRAP_S;
    GLint mData_GL_TEXTURE_WRAP_T;
    GLint mData_GL_TEXTURE_WRAP_R;
    GLfloat mData_GL_TEXTURE_BORDER_COLOR[4];
    GLfloat mData_GL_TEXTURE_PRIORITY;
    GLint mData_GL_TEXTURE_COMPARE_MODE;
    GLint mData_GL_TEXTURE_COMPARE_FUNC;
    GLint mData_GL_TEXTURE_SRGB_DECODE_EXT;
    GLint mData_GL_DEPTH_TEXTURE_MODE;
    GLint mData_GL_GENERATE_MIPMAP;

	GLfloat mData_GL_TEXTURE_MAX_ANISOTROPY_EXT;

	std::vector<TextureUpdateData> mSequentialUpdates;

	std::vector<TextureUpdateData> AppendTextureUpdate(const std::vector<TextureUpdateData>& _currentList, const TextureUpdateData& _update);
};

// ------------------------------------------------------------------------------------------------
class GLSampler
{
public:
	GLSampler(GLuint sampler=0);
	~GLSampler();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glBindSampler(GLuint unit, GLuint sampler);
	void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
	void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
	void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* params);

	GLuint Create(const GLTrace* _trace) const;

private:
	GLuint mSampler;

	GLint mData_GL_TEXTURE_MIN_FILTER;
    GLint mData_GL_TEXTURE_MAG_FILTER;
    GLfloat mData_GL_TEXTURE_MIN_LOD;
    GLfloat mData_GL_TEXTURE_MAX_LOD;
    GLint mData_GL_TEXTURE_WRAP_S;
    GLint mData_GL_TEXTURE_WRAP_T;
    GLint mData_GL_TEXTURE_WRAP_R;
    GLfloat mData_GL_TEXTURE_BORDER_COLOR[4];
    GLint mData_GL_TEXTURE_COMPARE_MODE;
    GLint mData_GL_TEXTURE_COMPARE_FUNC;
    GLint mData_GL_TEXTURE_SRGB_DECODE_EXT;

	GLfloat mData_GL_TEXTURE_MAX_ANISOTROPY_EXT;
};

