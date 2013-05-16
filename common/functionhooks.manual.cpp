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

#include "StdAfx.h"
#include <exception>

#include "functionhooks.manual.h"
#include "functionhooks.gen.h"

#include "common/common.h"
#include "common/extensions.h"

#include "common/gltrace.h"

bool gFirstMakeCurrent = true;

static void DummyFunc()
{
	// This will fail on Mac, leaving it here as a bomb to fix there. :|
	CompileTimeAssert(sizeof(GLhandleARB) == sizeof(GLuint));

	CompileTimeAssert(sizeof(GLcharARB) == sizeof(GLchar));
}

// ------------------------------------------------------------------------------------------------
GLenum TexImage2DTargetToBoundTarget(GLenum _target)
{
	switch (_target)
	{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: 
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: 
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: 
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: 
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: 
			return GL_TEXTURE_CUBE_MAP;
		default:
			break;
	};

	return _target;
}

// ------------------------------------------------------------------------------------------------
void GLClipPlane::Write(FileLike* _out) const
{
	_out->Write(mEquation[0]);
	_out->Write(mEquation[1]);
	_out->Write(mEquation[2]);
	_out->Write(mEquation[3]);
}

// ------------------------------------------------------------------------------------------------
void GLClipPlane::Read(FileLike* _in)
{
	_in->Read(&mEquation[0]);
	_in->Read(&mEquation[1]);
	_in->Read(&mEquation[2]);
	_in->Read(&mEquation[3]);
}

// ------------------------------------------------------------------------------------------------
inline size_t elementsToIndexCount(GLenum mode, GLsizei count)
{
	size_t retVal = 0;

	if (count == 0) { 
		return 0;
	}
	
	size_t startupIndices = 0;
	size_t perElementCost = 0;
	switch (mode)
	{
	case GL_POINTS:
		startupIndices = 1;
		perElementCost = 1;
		break;
	case GL_LINE_STRIP:
		startupIndices = 2;
		perElementCost = 1;
		break;
	case GL_LINE_LOOP: 
		startupIndices = 2;
		perElementCost = 1;
		break;
	case GL_LINES:
		startupIndices = 2;
		perElementCost = 2;
		break;
	case GL_TRIANGLE_STRIP: 
		startupIndices = 3;
		perElementCost = 1;
		break;
	case GL_TRIANGLE_FAN: 
		startupIndices = 3;
		perElementCost = 1;
		break;
	case GL_TRIANGLES: 
		startupIndices = 3;
		perElementCost = 3;
		break;
	case GL_QUAD_STRIP: 
		startupIndices = 4;
		perElementCost = 2;
		break;
	case GL_QUADS: 
		startupIndices = 4;
		perElementCost = 4;
		break;
	case GL_POLYGON:
		startupIndices = count;
		perElementCost = 0;
		break;
	default:
		assert(!"impl");
		Once(TraceError(TC("Error in elementsToIndexCount, couldn't convert mode='%d', count='%d' to elements."), mode, count));
	}

	return startupIndices + (count - 1) * perElementCost;
}

// ------------------------------------------------------------------------------------------------
inline size_t GLenumToSize(GLenum type)
{
	switch(type)
	{
	case GL_BYTE:
		return sizeof(GLbyte);
	case GL_UNSIGNED_BYTE:
		return sizeof(GLubyte);
	case GL_SHORT: 
		return sizeof(GLshort);
	case GL_UNSIGNED_SHORT: 
		return sizeof(GLushort);
	case GL_INT: 
		return sizeof(GLint);
	case GL_UNSIGNED_INT: 
		return sizeof(GLuint);
	case GL_FLOAT:
		return sizeof(GLfloat);
	case GL_DOUBLE:
		return sizeof(GLdouble);
	case GL_2_BYTES: 
		return 2;
	case GL_3_BYTES: 
		return 3;
	case GL_4_BYTES:
		return 4;
	default:
		assert(!"impl");
		Once(TraceError(TC("Don't know how to convert type='%d' to bytes."), type));
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t GLenumToParameterCount(GLenum pname)
{
	switch (pname) 
	{
	case GL_ACTIVE_TEXTURE: 
		return 1;
	case GL_ALIASED_LINE_WIDTH_RANGE: 
		return 2;
	case GL_ALIASED_POINT_SIZE_RANGE: 
		return 2;
	case GL_ALPHA_BITS: 
		return 1;
	case GL_ARRAY_BUFFER_BINDING: 
		return 1;
	case GL_BLEND: 
		return 1;
	case GL_BLEND_COLOR: 
		return 4;
	case GL_BLEND_DST_ALPHA: 
		return 1;
	case GL_BLEND_DST_RGB: 
		return 1;
	case GL_BLEND_EQUATION_ALPHA: 
		return 1;
	case GL_BLEND_EQUATION_RGB: 
		return 1;
	case GL_BLEND_SRC_ALPHA: 
		return 1;
	case GL_BLEND_SRC_RGB: 
		return 1;
	case GL_BLUE_BITS: 
		return 1;
	case GL_COLOR_CLEAR_VALUE: 
		return 4;
	case GL_COLOR_WRITEMASK: 
		return 4;
	case GL_COMPRESSED_TEXTURE_FORMATS: 
		assert(!"impl - todo"); // GL_NUM_COMPRESSED_TEXTURE_FORMATS
		Once(TraceError(TC("GLenumToParameterCount needs implementation for pname==GL_COMPRESSED_TEXTURE_FORMATS.")));
		return 0;
	case GL_CULL_FACE: 
		return 1;
	case GL_CULL_FACE_MODE: 
		return 1;
	case GL_CURRENT_PROGRAM: 
		return 1;
	case GL_DEPTH_BITS: 
		return 1;
	case GL_DEPTH_CLEAR_VALUE: 
		return 1;
	case GL_DEPTH_FUNC: 
		return 1;
	case GL_DEPTH_RANGE: 
		return 2;
	case GL_DEPTH_TEST: 
		return 1;
	case GL_DEPTH_WRITEMASK: 
		return 1;
	case GL_DITHER: 
		return 1;
	case GL_ELEMENT_ARRAY_BUFFER_BINDING: 
		return 1;
	case GL_FRAMEBUFFER_BINDING: 
		return 1;
	case GL_FRONT_FACE: 
		return 1;
	case GL_GENERATE_MIPMAP_HINT: 
		return 1;
	case GL_GREEN_BITS: 
		return 1;
	case GL_IMPLEMENTATION_COLOR_READ_FORMAT: 
		return 1;
	case GL_IMPLEMENTATION_COLOR_READ_TYPE: 
		return 1;
	case GL_LINE_WIDTH: 
		return 1;
	case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: 
		return 1;
	case GL_MAX_CUBE_MAP_TEXTURE_SIZE: 
		return 1;
	case GL_MAX_FRAGMENT_UNIFORM_VECTORS: 
		return 1;
	case GL_MAX_RENDERBUFFER_SIZE: 
		return 1;
	case GL_MAX_TEXTURE_IMAGE_UNITS: 
		return 1;
	case GL_MAX_TEXTURE_SIZE: 
		return 1;
	case GL_MAX_VARYING_VECTORS: 
		return 1;
	case GL_MAX_VERTEX_ATTRIBS: 
		return 1;
	case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: 
		return 1;
	case GL_MAX_VERTEX_UNIFORM_VECTORS: 
		return 1;
	case GL_MAX_VIEWPORT_DIMS: 
		return 2;
	case GL_NUM_COMPRESSED_TEXTURE_FORMATS: 
		return 1;
	case GL_NUM_SHADER_BINARY_FORMATS: 
		return 1;
	case GL_PACK_ALIGNMENT: 
		return 1;
	case GL_POLYGON_OFFSET_FACTOR: 
		return 1;
	case GL_POLYGON_OFFSET_FILL: 
		return 1;
	case GL_POLYGON_OFFSET_UNITS: 
		return 1;
	case GL_RED_BITS: 
		return 1;
	case GL_RENDERBUFFER_BINDING: 
		return 1;
	case GL_SAMPLE_ALPHA_TO_COVERAGE: 
		return 1;
	case GL_SAMPLE_BUFFERS: 
		return 1;
	case GL_SAMPLE_COVERAGE: 
		return 1;
	case GL_SAMPLE_COVERAGE_INVERT: 
		return 1;
	case GL_SAMPLE_COVERAGE_VALUE: 
		return 1;
	case GL_SAMPLES: 
		return 1;
	case GL_SCISSOR_BOX: 
		return 4;
	case GL_SCISSOR_TEST: 
		return 1;
	// case GL_SHADER_BINARY_FORMATS: 
	//	assert(!"impl - todo"); // GL_NUM_SHADER_BINARY_FORMATS
	//	return 0;
	case GL_SHADER_COMPILER: 
		return 1;
	case GL_STENCIL_BACK_FAIL: 
		return 1;
	case GL_STENCIL_BACK_FUNC: 
		return 1;
	case GL_STENCIL_BACK_PASS_DEPTH_FAIL: 
		return 1;
	case GL_STENCIL_BACK_PASS_DEPTH_PASS: 
		return 1;
	case GL_STENCIL_BACK_REF: 
		return 1;
	case GL_STENCIL_BACK_VALUE_MASK: 
		return 1;
	case GL_STENCIL_BACK_WRITEMASK: 
		return 1;
	case GL_STENCIL_BITS: 
		return 1;
	case GL_STENCIL_CLEAR_VALUE: 
		return 1;
	case GL_STENCIL_FAIL: 
		return 1;
	case GL_STENCIL_FUNC: 
		return 1;
	case GL_STENCIL_PASS_DEPTH_FAIL: 
		return 1;
	case GL_STENCIL_PASS_DEPTH_PASS: 
		return 1;
	case GL_STENCIL_REF: 
		return 1;
	case GL_STENCIL_TEST: 
		return 1;
	case GL_STENCIL_VALUE_MASK: 
		return 1;
	case GL_STENCIL_WRITEMASK: 
		return 1;
	case GL_SUBPIXEL_BITS: 
		return 1;
	case GL_TEXTURE_BINDING_2D: 
		return 1;
	case GL_TEXTURE_BINDING_CUBE_MAP: 
		return 1;
	case GL_UNPACK_ALIGNMENT:
		return 4;
	case GL_VIEWPORT: 
		return 4;

	// For lighting
	case GL_AMBIENT:
		return 4;
	case GL_DIFFUSE:
		return 4;
	case GL_SPECULAR:
		return 4;
	case GL_EMISSION:
		return 4;
	case GL_POSITION:
		return 4;
	case GL_SPOT_DIRECTION:
		return 3;
	case GL_SPOT_EXPONENT:
		return 1;
	case GL_SPOT_CUTOFF:
		return 1;
	case GL_CONSTANT_ATTENUATION:
		return 1;
	case GL_LINEAR_ATTENUATION:
		return 1;
	case GL_QUADRATIC_ATTENUATION:
		return 1;
	case GL_SHININESS:
		return 1;
	case GL_COLOR_INDEXES:
		return 3;

	default:
		assert(!"impl");
		Once(TraceError(TC("GLenumToParameterCount needs implementation for pname='%d'."), pname));
		return 0;
	};

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t formatAndTypeToSizePerPixel(GLenum _format, GLenum _type)
{
	GLsizei srcBytesPerComponent = 0;
	switch (_type)
	{
		case GL_UNSIGNED_BYTE:
			srcBytesPerComponent = sizeof(GLubyte);
			break;
		case GL_BYTE: 
			srcBytesPerComponent = sizeof(GLbyte);
			break;
		case GL_UNSIGNED_SHORT: 
			srcBytesPerComponent = sizeof(GLushort);
			break;
		case GL_SHORT:
			srcBytesPerComponent = sizeof(GLshort);
			break;
		case GL_UNSIGNED_INT:
			srcBytesPerComponent = sizeof(GLuint);
			break;
		case GL_INT:
			srcBytesPerComponent = sizeof(GLint);
			break;
		case GL_FLOAT:
			srcBytesPerComponent = sizeof(GLfloat);
			break;
		case GL_HALF_FLOAT:
			srcBytesPerComponent = sizeof(GLhalfARB);
			break;

		// NOTE: These early return
		case GL_UNSIGNED_BYTE_3_3_2:
			return sizeof(GLubyte);
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			return sizeof(GLubyte);
		case GL_UNSIGNED_SHORT_5_6_5:
			return sizeof(GLushort);
		case GL_UNSIGNED_SHORT_5_6_5_REV:
			return sizeof(GLushort);
		case GL_UNSIGNED_SHORT_4_4_4_4:
			return sizeof(GLushort);
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			return sizeof(GLushort);
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return sizeof(GLushort);
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			return sizeof(GLushort);
		case GL_UNSIGNED_INT_8_8_8_8:
			return sizeof(GLuint);
		case GL_UNSIGNED_INT_8_8_8_8_REV:
			return sizeof(GLuint);
		case GL_UNSIGNED_INT_10_10_10_2:
			return sizeof(GLuint);
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			return sizeof(GLuint);
		case GL_BITMAP: 
		default:
			assert(!"Don't know how to deal with this type parameter.");
			Once(TraceError(TC("formatAndTypeToSizePerPixel - Don't know how to deal with type='%d'"), _type)); 
			break;
	};


	GLsizei srcComponents = 0;
	switch (_format) 
	{
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
			srcComponents = 1;
			break;
		case GL_RGB:
		case GL_BGR:
			srcComponents = 3;
			break;
		case GL_RGBA:
		case GL_BGRA:
			srcComponents = 4;
			break;
		case GL_LUMINANCE:
			srcComponents = 1;
			break;
		case GL_LUMINANCE_ALPHA:
			srcComponents = 2;
			break;
		case GL_DEPTH_COMPONENT:
			srcComponents = 1;
			break;
		case GL_COLOR_INDEX:
			// Don't know what to do with these, need to ask.
		default:
			assert(!"Unimplemented glTexImage1D format");
			Once(TraceError(TC("formatAndTypeToSizePerPixel - Don't know how to deal with format='%d'"), _format)); 
	};

	return srcBytesPerComponent * srcComponents;
}

// ------------------------------------------------------------------------------------------------
inline bool IsValidTarget_glBindBuffer(GLenum _target)
{
	switch (_target) 
	{
		case GL_ARRAY_BUFFER:
		case GL_ATOMIC_COUNTER_BUFFER: 
		case GL_COPY_READ_BUFFER:
		case GL_COPY_WRITE_BUFFER: 
		case GL_DRAW_INDIRECT_BUFFER: 
		case GL_ELEMENT_ARRAY_BUFFER: 
		case GL_PIXEL_PACK_BUFFER: 
		case GL_PIXEL_UNPACK_BUFFER:
		case GL_TEXTURE_BUFFER: 
		case GL_TRANSFORM_FEEDBACK_BUFFER: 
		case GL_UNIFORM_BUFFER:
			return true;

		default:
			break;
	};

	return false;
};

bool (*IsValidTarget_glBufferData)(GLenum _target) = IsValidTarget_glBindBuffer;
bool (*IsValidTarget_glBufferDataARB)(GLenum _target) = IsValidTarget_glBindBuffer;
bool (*IsValidTarget_glBufferSubData)(GLenum _target) = IsValidTarget_glBindBuffer;

// ------------------------------------------------------------------------------------------------
bool IsValidTarget_glBindProgramARB(GLenum _target)
{
	return _target == GL_VERTEX_PROGRAM_ARB 
		|| _target == GL_FRAGMENT_PROGRAM_ARB;
}

// ------------------------------------------------------------------------------------------------
bool IsValidTarget_glFlushMappedBufferRange(GLenum _target)
{
	if (IsValidTarget_glBindBuffer(_target)) { 
		switch (_target)
		{
			case GL_ARRAY_BUFFER:
			case GL_COPY_READ_BUFFER:
			case GL_COPY_WRITE_BUFFER: 
			case GL_ELEMENT_ARRAY_BUFFER: 
			case GL_PIXEL_PACK_BUFFER: 
			case GL_PIXEL_UNPACK_BUFFER: 
			case GL_TEXTURE_BUFFER: 
			case GL_TRANSFORM_FEEDBACK_BUFFER: 
			case GL_UNIFORM_BUFFER:
				return true;
			default:
				break;
		};
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
inline bool IsValidBufferUsage(GLenum _usage)
{
	switch (_usage) 
	{
		case GL_STREAM_DRAW:
		case GL_STREAM_READ:
		case GL_STREAM_COPY:
		case GL_STATIC_DRAW: 
		case GL_STATIC_READ: 
		case GL_STATIC_COPY: 
		case GL_DYNAMIC_DRAW: 
		case GL_DYNAMIC_READ: 
		case GL_DYNAMIC_COPY:
			return true;

		default:
			break;
	};

	return false;
}

// ------------------------------------------------------------------------------------------------
size_t determineTexImageBufferSize(GLsizei _widthPixels, GLsizei _heightPixels, GLsizei _depthPixels, GLenum _format, GLenum _type, 
                                   GLint _unpackAlignment, GLint _unpackRowLength, GLint _unpackImageHeight, 
								   GLint _unpackSkipPixels, GLint _unpackSkipRows, GLint _unpackSkipImages)
{
	if (_widthPixels == 0 || _heightPixels == 0 || _depthPixels == 0) {
		assert(0);
		Once(TraceError(TC("determineTexImageBufferSize called with wonky parameters--could be valid, please check.")));
		Once(TraceError(TC("determineTexImageBufferSize(_widthPixels=%d, _heightPixels=%d, _depthPixels=%d, _format=%d, _type=%d, _unpackAlignment=%d, _unpackRowLength=%d, _unpackImageHeight=%d, _unpackSkipPixels=%d, _unpackSkipRows=%d, _unpackSkipImages=%d)"), 
					    _widthPixels, _heightPixels, _depthPixels, _format, _type, _unpackAlignment, _unpackRowLength, _unpackImageHeight, _unpackSkipPixels, _unpackSkipRows, _unpackSkipImages));
		return 0;
	}

	size_t sizePerPixel = formatAndTypeToSizePerPixel(_format, _type);
	if (sizePerPixel == 0) {
		return 0;
	}

	GLsizei rowWidthPixels = (_unpackRowLength > 0 ? _unpackRowLength : _widthPixels);
	GLsizei rowWidthBytes = iceil(sizePerPixel * rowWidthPixels, size_t(_unpackAlignment));
	GLsizei imageSizeBytes = rowWidthBytes * (_unpackImageHeight > 0 ? _unpackImageHeight : _heightPixels);

	// From the start of a single row to the left-most pixel we want on that row.
	GLsizei pixelOffsetBytes = sizePerPixel * (_unpackSkipPixels > 0 ? _unpackSkipPixels : 0);
	// From the top of an image to the top-most pixel we want on that page.
	GLsizei rowOffsetBytes = rowWidthBytes * (_unpackSkipRows > 0 ? _unpackSkipRows : 0);
	// From the pointer passed in to the first texture we want data from.
	GLsizei imageOffsetBytes = imageSizeBytes * (_unpackSkipImages > 0 ? _unpackSkipImages : 0);

	// Number of bytes of data to pick up from the final row.
	// Note this uses passed-in width and ignores _unpackRowLength.
	GLsizei trailingRowBytes = iceil(sizePerPixel * _widthPixels, size_t(_unpackAlignment));

	GLsizei firstInterestingByte = pixelOffsetBytes + rowOffsetBytes + imageOffsetBytes;

	return firstInterestingByte
		 + ((_depthPixels - 1) * imageSizeBytes)
		 + ((_heightPixels - 1) * rowWidthBytes)
		 + trailingRowBytes;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBitmap_bitmap(const ContextState* _ctxState, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap)
{
	assert(!"impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glBitmap_bitmap"))); 

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glCallLists_lists(const ContextState* _ctxState, GLsizei n, GLenum type, const GLvoid* lists)
{
	if (!lists) 
		return 0;
	return n * GLenumToSize(type);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glClipPlane_equation(const ContextState* _ctxState, GLenum plane, const GLdouble* equation)
{
	if (!equation) 
		return 0;
	return 4 * sizeof(GLdouble);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glColorPointer_pointer(const ContextState* _ctxState, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glColorPointer with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glCompressedTexImage2D_data(const ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data)
{
	return imagesize * sizeof(GLubyte);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glCompressedTexImage3D_data(const ContextState* _ctxState, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid* data)
{
	return imagesize * sizeof(GLubyte);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glDrawElements_indices(const ContextState* _ctxState, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
	if (!indices) 
		return 0;
	size_t indexCount = elementsToIndexCount(mode, count);	
	return indexCount * GLenumToSize(type);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glDrawPixels_pixels(const ContextState* _ctxState, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	if (!pixels)
		return 0;
	assert(!"lolimpl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glDrawPixels_pixels"))); 
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glDrawRangeElements_indices(const ContextState* _ctxState, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ELEMENT_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glDrawRangeElements with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glDrawRangeElementsBaseVertex_indices(const ContextState* _ctxState, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ELEMENT_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glDrawRangeElementsBaseVertex with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glEdgeFlagPointer_pointer(const ContextState* _ctxState, GLsizei stride, const GLvoid* pointer)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glEdgeFlagPointer with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;	
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glFeedbackBuffer_buffer(const ContextState* _ctxState, GLsizei size, GLenum type, GLfloat* buffer)
{
	assert(!"feedback buffers are totally not implemented.");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glFeedbackBuffer_buffer")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetClipPlane_equation(const ContextState* _ctxState, GLenum plane, GLdouble* equation)
{
	return 4 * sizeof(GLdouble);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetMapdv_v(const ContextState* _ctxState, GLenum target, GLenum query, GLdouble* v)
{
	assert(!"glGetMapdv - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetMapdv_v")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetMapfv_v(const ContextState* _ctxState, GLenum target, GLenum query, GLfloat* v)
{
	assert(!"glGetMapdv - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetMapfv_v")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetMapiv_v(const ContextState* _ctxState, GLenum target, GLenum query, GLint* v)
{
	assert(!"glGetMapiv - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetMapiv_v")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetPixelMapfv_values(const ContextState* _ctxState, GLenum map, GLfloat* values)
{
	assert(!"glGetPixelMap - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetPixelMapfv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetPixelMapuiv_values(const ContextState* _ctxState, GLenum map, GLuint* values)
{
	assert(!"glGetPixelMap - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetPixelMapuiv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetPixelMapusv_values(const ContextState* _ctxState, GLenum map, GLushort* values)
{
	assert(!"glGetPixelMap - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetPixelMapusv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetPointerv_params(const ContextState* _ctxState, GLenum pname, GLvoid** params)
{
	// Errr, this isn't right.
	assert(!"glGetPointerv - impl");
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented (well)."), TC("determinePointerLength_glGetPointerv_params")));
	return sizeof(GLvoid*);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetPolygonStipple_mask(const ContextState* _ctxState, GLubyte* mask)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetPolygonStipple_mask")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetShaderiv_params(const ContextState* _ctxState, GLuint shader, GLenum pname, GLint* params)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetProgramiv_params(const ContextState* _ctxState, GLuint program, GLenum pname, GLint* params)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexEnvfv_params(const ContextState* _ctxState, GLenum target, GLenum pname, GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexEnvfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexEnviv_params(const ContextState* _ctxState, GLenum target, GLenum pname, GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexEnviv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexGendv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, GLdouble* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexGendv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexGenfv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexGenfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexGeniv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexGeniv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexImage_pixels(const ContextState* _ctxState, GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexImage_pixels")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexLevelParameterfv_params(const ContextState* _ctxState, GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexLevelParameterfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexLevelParameteriv_params(const ContextState* _ctxState, GLenum target, GLint level, GLenum pname, GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexLevelParameteriv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexParameterfv_params(const ContextState* _ctxState, GLenum target, GLenum pname, GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexParameterfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexParameteriv_params(const ContextState* _ctxState, GLenum target, GLenum pname, GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexParameteriv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetUniformLocation_name(const ContextState* _ctxState, GLuint program, const GLchar* name)
{
	return strlen(name) + 1;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glInterleavedArrays_pointer(const ContextState* _ctxState, GLenum format, GLsizei stride, const GLvoid* pointer)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glInterleavedArrays_pointer")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glLightModelfv_params(const ContextState* _ctxState, GLenum pname, const GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glLightModelfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glLightModeliv_params(const ContextState* _ctxState, GLenum pname, const GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glLightModeliv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glLightfv_params(const ContextState* _ctxState, GLenum light, GLenum pname, const GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glLightfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glLightiv_params(const ContextState* _ctxState, GLenum light, GLenum pname, const GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glLightiv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMap1d_points(const ContextState* _ctxState, GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMap1d_points")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMap1f_points(const ContextState* _ctxState, GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMap1f_points")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMap2d_points(const ContextState* _ctxState, GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMap2d_points")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMap2f_points(const ContextState* _ctxState, GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMap2f_points")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMaterialfv_params(const ContextState* _ctxState, GLenum face, GLenum pname, const GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMaterialfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glMaterialiv_params(const ContextState* _ctxState, GLenum face, GLenum pname, const GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glMaterialiv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glPixelMapfv_values(const ContextState* _ctxState, GLenum map, GLsizei mapsize, const GLfloat* values)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glPixelMapfv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glPixelMapuiv_values(const ContextState* _ctxState, GLenum map, GLsizei mapsize, const GLuint* values)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glPixelMapuiv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glPixelMapusv_values(const ContextState* _ctxState, GLenum map, GLsizei mapsize, const GLushort* values)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glPixelMapusv_values")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glPolygonStipple_mask(const ContextState* _ctxState, const GLubyte* mask)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glPolygonStipple_mask")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glProgramEnvParameters4fvEXT_params(const ContextState* _ctxState, GLenum target, GLuint index, GLsizei count, const GLfloat* params)
{
	return 4 * count * sizeof(GLfloat);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glReadPixels_pixels(const ContextState* _ctxState, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glReadPixels_pixels")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glSamplerParameterfv_params(const ContextState* _ctxState, GLuint sampler, GLenum pname, const GLfloat* params)
{
	switch (pname)
	{
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	case GL_TEXTURE_WRAP_R: 
	case GL_TEXTURE_MIN_FILTER: 
	case GL_TEXTURE_MAG_FILTER:
	case GL_TEXTURE_MIN_LOD: 
	case GL_TEXTURE_MAX_LOD: 
	case GL_TEXTURE_LOD_BIAS: 
	case GL_TEXTURE_COMPARE_MODE: 
	case GL_TEXTURE_COMPARE_FUNC:
		return sizeof(GLfloat);

	case GL_TEXTURE_BORDER_COLOR:
		return 4 * sizeof(GLfloat);

	default:
		assert(0);
		Once(TraceError(TC("Frame called glSamplerParameter with pname==%d, but don't know argument size for that--update glTrace"), pname));
	};

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glSelectBuffer_buffer(const ContextState* _ctxState, GLsizei size, GLuint* buffer)
{
	if (!buffer)
		return 0;
	return (size_t)(size * sizeof(GLuint));
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glShaderSource_string(const ContextState* _ctxState, GLuint shader, GLsizei count, const GLcharARB** string, const GLint* length)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glShaderSource_length(const ContextState* _ctxState, GLuint shader, GLsizei count, const GLcharARB** string, const GLint* length)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glShaderSourceARB_string(const ContextState* _ctxState, GLuint shader, GLsizei count, const GLcharARB** string, const GLint* length)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glShaderSourceARB_length(const ContextState* _ctxState, GLuint shader, GLsizei count, const GLcharARB** string, const GLint* length)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexCoordPointer_pointer(const ContextState* _ctxState, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glTexCoordPointer with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexEnvfv_params(const ContextState* _ctxState, GLenum target, GLenum pname, const GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTexEnvfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexEnviv_params(const ContextState* _ctxState, GLenum target, GLenum pname, const GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTexEnviv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexGendv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, const GLdouble* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTexGendv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexGenfv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, const GLfloat* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTexGenfv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexGeniv_params(const ContextState* _ctxState, GLenum coord, GLenum pname, const GLint* params)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTexGeniv_params")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexImage1D_pixels(const ContextState* _ctxState, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	auto data_GL_UNPACK_ROW_LENGTH = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ROW_LENGTH);
	auto data_GL_UNPACK_IMAGE_HEIGHT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_IMAGE_HEIGHT);
	auto data_GL_UNPACK_SKIP_PIXELS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_PIXELS);
	auto data_GL_UNPACK_SKIP_ROWS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_ROWS);
	auto data_GL_UNPACK_ALIGNMENT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ALIGNMENT);

	return determineTexImageBufferSize(width, 1, 1, format, type, 
	                                   data_GL_UNPACK_ALIGNMENT, data_GL_UNPACK_ROW_LENGTH, data_GL_UNPACK_IMAGE_HEIGHT, 
	                                   data_GL_UNPACK_SKIP_PIXELS, data_GL_UNPACK_SKIP_ROWS, 0);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexImage2D_pixels(const ContextState* _ctxState, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	auto data_GL_UNPACK_ROW_LENGTH = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ROW_LENGTH);
	auto data_GL_UNPACK_IMAGE_HEIGHT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_IMAGE_HEIGHT);
	auto data_GL_UNPACK_SKIP_PIXELS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_PIXELS);
	auto data_GL_UNPACK_SKIP_ROWS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_ROWS);
	auto data_GL_UNPACK_ALIGNMENT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ALIGNMENT);

	return determineTexImageBufferSize(width, height, 1, format, type, 
	                                   data_GL_UNPACK_ALIGNMENT, data_GL_UNPACK_ROW_LENGTH, data_GL_UNPACK_IMAGE_HEIGHT, 
	                                   data_GL_UNPACK_SKIP_PIXELS, data_GL_UNPACK_SKIP_ROWS, 0);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexParameterfv_params(const ContextState* _ctxState, GLenum target, GLenum pname, const GLfloat* params)
{
	switch(pname) 
	{
	case GL_TEXTURE_MIN_FILTER:
	case GL_TEXTURE_MAG_FILTER:
	case GL_TEXTURE_MIN_LOD:
	case GL_TEXTURE_MAX_LOD:
	case GL_TEXTURE_BASE_LEVEL:
	case GL_TEXTURE_MAX_LEVEL:
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	case GL_TEXTURE_WRAP_R:
	case GL_TEXTURE_PRIORITY:
	case GL_TEXTURE_COMPARE_MODE:
	case GL_TEXTURE_COMPARE_FUNC:
	case GL_DEPTH_TEXTURE_MODE:
	case GL_GENERATE_MIPMAP:
		return 1 * sizeof(GLfloat);
	case GL_TEXTURE_BORDER_COLOR:
		return 4 * sizeof(GLfloat);
	default:
		assert(!"impl");
		Once(TraceError(TC("Frame called '%s' with pname='%d', which isn't implemented. Please implement."), TC("determinePointerLength_glTexParameterfv_params"), pname));

		break;
	};
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexParameteriv_params(const ContextState* _ctxState, GLenum target, GLenum pname, const GLint* params)
{
	switch(pname) 
	{
	case GL_TEXTURE_MIN_FILTER:
	case GL_TEXTURE_MAG_FILTER:
	case GL_TEXTURE_MIN_LOD:
	case GL_TEXTURE_MAX_LOD:
	case GL_TEXTURE_BASE_LEVEL:
	case GL_TEXTURE_MAX_LEVEL:
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	case GL_TEXTURE_WRAP_R:
	case GL_TEXTURE_PRIORITY:
	case GL_TEXTURE_COMPARE_MODE:
	case GL_TEXTURE_COMPARE_FUNC:
	case GL_DEPTH_TEXTURE_MODE:
	case GL_GENERATE_MIPMAP:
		return 1 * sizeof(GLint);
	case GL_TEXTURE_BORDER_COLOR:
		return 4 * sizeof(GLint);
	default:
		assert(!"impl");
		break;
	};
	return 0;}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexSubImage1D_pixels(const ContextState* _ctxState, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels)
{
	auto data_GL_UNPACK_ROW_LENGTH = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ROW_LENGTH);
	auto data_GL_UNPACK_IMAGE_HEIGHT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_IMAGE_HEIGHT);
	auto data_GL_UNPACK_SKIP_PIXELS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_PIXELS);
	auto data_GL_UNPACK_SKIP_ROWS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_ROWS);
	auto data_GL_UNPACK_ALIGNMENT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ALIGNMENT);

	return determineTexImageBufferSize(width, 1, 1, format, type, 
	                                   data_GL_UNPACK_ALIGNMENT, data_GL_UNPACK_ROW_LENGTH, data_GL_UNPACK_IMAGE_HEIGHT, 
	                                   data_GL_UNPACK_SKIP_PIXELS, data_GL_UNPACK_SKIP_ROWS, 0);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexSubImage2D_pixels(const ContextState* _ctxState, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	auto data_GL_UNPACK_ROW_LENGTH = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ROW_LENGTH);
	auto data_GL_UNPACK_IMAGE_HEIGHT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_IMAGE_HEIGHT);
	auto data_GL_UNPACK_SKIP_PIXELS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_PIXELS);
	auto data_GL_UNPACK_SKIP_ROWS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_ROWS);
	auto data_GL_UNPACK_ALIGNMENT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ALIGNMENT);

	return determineTexImageBufferSize(width, height, 1, format, type, 
	                                   data_GL_UNPACK_ALIGNMENT, data_GL_UNPACK_ROW_LENGTH, data_GL_UNPACK_IMAGE_HEIGHT, 
	                                   data_GL_UNPACK_SKIP_PIXELS, data_GL_UNPACK_SKIP_ROWS, 0);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glVertexAttribPointer_pointer(const ContextState* _ctxState, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glVertexAttribPointer with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glVertexPointer_pointer(const ContextState* _ctxState, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	const auto& bufferBindings = _ctxState->GetBufferBindings();
	auto it = bufferBindings.find(GL_ARRAY_BUFFER);
	if (it == bufferBindings.end() || it->second == NULL) {
		Once(TraceError(TC("Capture is using glVertexPointer with client memory pointers--this is not yet implemented. Trace replay probably corrupted.")));
		return 0;
	}

	// Not a pointer--it's an offset.
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBindAttribLocation_name(const ContextState* _ctxState, GLuint program, GLuint index, const GLchar* name)
{
	if (name == NULL) {
		return 0;
	}

	return strlen(name) + 1;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBindAttribLocationARB_name(const ContextState* _ctxState, GLhandleARB program, GLuint index, const GLcharARB* name)
{
	return determinePointerLength_glBindAttribLocation_name(_ctxState, program, index, name);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBufferData_data(const ContextState* _ctxState, GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage)
{
	if (data) { 
		return size;
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBufferDataARB_data(const ContextState* _ctxState, GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage)
{
	return determinePointerLength_glBufferData_data(_ctxState, target, size, data, usage);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glBufferSubData_data(const ContextState* _ctxState, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
	if (data) {
		return size;
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glCompressedTexImage2D_h(const ContextState* _ctxState, GLenum a, GLint b, GLenum c, GLsizei d, GLsizei e, GLint f, GLsizei g, const GLvoid* h)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glCompressedTexImage2D_h")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glCompressedTexImage3D_i(const ContextState* _ctxState, GLenum a, GLint b, GLenum c, GLsizei d, GLsizei e, GLsizei f, GLint g, GLsizei h, const GLvoid* i)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glCompressedTexImage3D_i")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glDrawRangeElements_f(const ContextState* _ctxState, GLenum a, GLuint b, GLuint c, GLsizei d, GLenum e, const GLvoid* f)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glDrawRangeElements_f")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetBooleanIndexedvEXT_c(const ContextState* _ctxState, GLenum a, GLuint b, GLboolean* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetBooleanIndexedvEXT_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetCompressedTexImage_c(const ContextState* _ctxState, GLenum a, GLint b, GLvoid* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetCompressedTexImage_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetObjectParameterivARB_c(const ContextState* _ctxState, GLhandleARB a, GLenum b, GLint* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetObjectParameterivARB_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetProgramivARB_c(const ContextState* _ctxState, GLenum a, GLenum b, GLint* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetProgramivARB_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetQueryObjectivARB_params(const ContextState* _ctxState, GLuint id, GLenum pname, GLint* params)
{
	return sizeof(GLint);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetQueryObjectuivARB_params(const ContextState* _ctxState, GLuint id, GLenum pname, GLuint* params)
{
	return sizeof(GLuint);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetSynciv_d(const ContextState* _ctxState, GLsync a, GLenum b, GLsizei c, GLsizei* d, GLint* e)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetSynciv_d")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetSynciv_e(const ContextState* _ctxState, GLsync a, GLenum b, GLsizei c, GLsizei* d, GLint* e)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetSynciv_e")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glGetTexParameterPointervAPPLE_c(const ContextState* _ctxState, GLenum a, GLenum b, void* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glGetTexParameterPointervAPPLE_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glProgramStringARB_string(const ContextState* _ctxState, GLenum target, GLenum format, GLsizei len, const GLvoid* string)
{
	if (string) {
		assert(len == strlen((GLchar*)string));
		return len + 1;
	}
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glStringMarkerGREMEDY_b(const ContextState* _ctxState, GLsizei a, const void* b)
{
	return a * sizeof(GLchar);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTexImage3D_data(const ContextState* _ctxState, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
	auto data_GL_UNPACK_ROW_LENGTH = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ROW_LENGTH);
	auto data_GL_UNPACK_IMAGE_HEIGHT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_IMAGE_HEIGHT);
	auto data_GL_UNPACK_SKIP_PIXELS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_PIXELS);
	auto data_GL_UNPACK_SKIP_ROWS = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_ROWS);
	auto data_GL_UNPACK_SKIP_IMAGES = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_SKIP_IMAGES);
	auto data_GL_UNPACK_ALIGNMENT = _ctxState->GetPixelStoreState().glGet<GLint>(GL_UNPACK_ALIGNMENT);

	return determineTexImageBufferSize(width, height, depth, format, type, 
	                                   data_GL_UNPACK_ALIGNMENT, data_GL_UNPACK_ROW_LENGTH, data_GL_UNPACK_IMAGE_HEIGHT, 
	                                   data_GL_UNPACK_SKIP_PIXELS, data_GL_UNPACK_SKIP_ROWS, data_GL_UNPACK_SKIP_IMAGES);
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glTextureRangeAPPLE_c(const ContextState* _ctxState, GLenum a, GLsizei b, void* c)
{
	Once(TraceError(TC("Frame tried to call '%s', which is currently not implemented."), TC("determinePointerLength_glTextureRangeAPPLE_c")));
	return 0;
}

// ------------------------------------------------------------------------------------------------
size_t determinePointerLength_glUniform4fv_value(const ContextState* _ctxState, GLint location, GLsizei count, const GLfloat* value)
{
	if (!value)
		return 0;
	return (size_t)(4 * count * sizeof(GLfloat));
}

// ------------------------------------------------------------------------------------------------
void OnCaptureStart(FileLike* _out)
{
	gIsRecording = true; 
	_out->Write(Checkpoint("TraceCapturingBegin"));
	_out->Write(*gContextState);
	_out->Write(Checkpoint("FrameCommandsBegin"));
}

// ------------------------------------------------------------------------------------------------
void OnCaptureEnd(FileLike* _out)
{
	// Send over trace logging messages
	WriteMessages(_out);

	// Send our "we're done sending messages for now" packet.
	SSerializeDataPacket pkt;
	pkt.mDataType = EST_Sentinel;
	_out->Write(pkt);
	_out->Write(Checkpoint("FrameCommandsEnd"));
	_out->Write(Checkpoint("TraceCapturingEnd"));
	gIsRecording = false;
}

// ------------------------------------------------------------------------------------------------
BOOL APIENTRY hooked_SwapBuffers(HDC hdc)
{
	assert(gMessageStream != 0);

	// TODO: This should go into a global or something.
	FileLike likeSocket(gMessageStream);
	
	if (gIsRecording) {
		SSerializeDataPacket::glFinish().Write(&likeSocket);

		OnCaptureEnd(&likeSocket);
	}

	RemoteCommand rc;
	if (gMessageStream->Recv(&rc, sizeof(rc))) {
		switch (rc.mRemoteCommandType) {
		case ERC_Capture: 
			OnCaptureStart(&likeSocket);
			break;
		case ERC_Terminate:
			PostQuitMessage(0);
			break;
		default:
			assert(0);
		};
	}

	return gReal_SwapBuffers(hdc);
}

// ------------------------------------------------------------------------------------------------
void APIENTRY hooked_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	if (gIsRecording)
		SSerializeDataPacket::glFlushMappedBufferRange(target, offset, length).Write(&FileLike(gMessageStream));
	gContextState->glFlushMappedBufferRange(target, offset, length);

	// Call the real function after we've updated the buffer contents.
	gReal_glFlushMappedBufferRange(target, offset, length);
}

// ------------------------------------------------------------------------------------------------
GLvoid* APIENTRY hooked_glMapBufferARB(GLenum target, GLenum access)
{
	auto retVal = gReal_glMapBufferARB(target, access);
	if (gIsRecording)
		SSerializeDataPacket::glMapBufferARB(target, access).Write(&FileLike(gMessageStream));
	
	return gContextState->glMapBufferARB(retVal, target, access);
}

// ------------------------------------------------------------------------------------------------
GLvoid* APIENTRY hooked_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	auto retVal = gReal_glMapBufferRange(target, offset, length, access);
	if (gIsRecording)
		SSerializeDataPacket::glMapBufferRange(target, offset, length, access).Write(&FileLike(gMessageStream));

	return gContextState->glMapBufferRange(retVal, target, offset, length, access);
}

// ------------------------------------------------------------------------------------------------
GLboolean APIENTRY hooked_glUnmapBuffer(GLenum buffer)
{
	// This is manual because during unmap we have to call the state-tracking version first to let 
	// it have a crack at updating buffers.
	if (gIsRecording)
		SSerializeDataPacket::glUnmapBuffer(buffer).Write(&FileLike(gMessageStream));
	gContextState->glUnmapBuffer(true, buffer);
	return gReal_glUnmapBuffer(buffer);
}

// ------------------------------------------------------------------------------------------------
BOOL APIENTRY hooked_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	BOOL retVal = gReal_wglMakeCurrent(hdc, hglrc);
	
	if (gFirstMakeCurrent) {
		Generated_ResolveDynamics();
		Generated_AttachDynamicHooks();

		gFirstMakeCurrent = false;
	}

	if (hglrc != NULL && retVal) {
		gContextState->SetOwnerThreadId(GetCurrentThreadId());
	} else {
		gContextState->SetOwnerThreadId(0);
	}

	return retVal;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ContextState::OnCaptureStart()
{
	// Nothing here atm.
}

// ------------------------------------------------------------------------------------------------
void ContextState::SetOwnerThreadId(DWORD _threadId)
{
	// If trying to debug MakeCurrent bugs, uncomment this.
	// assert(((_threadId == 0) != (mData_OwnerThread == 0)) || mData_OwnerThread == _threadId);
	mData_OwnerThread = _threadId;
}

// ------------------------------------------------------------------------------------------------
bool ContextState::CheckOwnerThreadId() const
{
	DWORD curThreadId = GetCurrentThreadId();
	return mData_OwnerThread == curThreadId;
}


// ------------------------------------------------------------------------------------------------
void ContextState::ManualConstruct()
{
	// @TODO: Create default textures, stick them in each of the 0 slots.
	mData_OwnerThread = 0;

	mData_DrawBuffer = GL_NONE;
	mData_ReadBuffer = GL_NONE;
}

// ------------------------------------------------------------------------------------------------
void ContextState::ManualDestruct()
{
	for (auto it = mData_TextureObjects.begin(); it != mData_TextureObjects.end(); ++it) {
		SafeDelete(it->second);
	}
	mData_TextureObjects.clear();
}

// ------------------------------------------------------------------------------------------------
void ContextState::ManualWrite(FileLike* _out) const
{
	_out->Write(Checkpoint("ContextStateBegin"));

	_out->Write(Checkpoint("TexturesBegin"));
	_out->Write(mData_TextureObjects);
	_out->Write(mData_TextureUnits);
	_out->Write(Checkpoint("TexturesEnd"));

	_out->Write(mData_PixelStoreState);
	_out->Write(mData_PixelTransferState);

	_out->Write(Checkpoint("BuffersBegin"));
	_out->Write(mData_BufferObjects);
	_out->Write(mData_BufferBindings);
	_out->Write(Checkpoint("BuffersEnd"));

	_out->Write(Checkpoint("ShadersBegin"));
	_out->Write(mData_ShaderObjectsGLSL);
	_out->Write(Checkpoint("ShadersEnd"));

	_out->Write(Checkpoint("ProgramsBegin"));
	_out->Write(mData_ProgramObjectsGLSL);
	_out->Write(Checkpoint("ProgramsEnd"));

	_out->Write(Checkpoint("ProgramsARBBegin"));
	_out->Write(mData_ProgramBindingsARB);
	_out->Write(mData_ProgramObjectsARB);
	_out->Write(Checkpoint("ProgramsARBEnd"));

	_out->Write(Checkpoint("EnableCapsBegin"));
	_out->Write(mData_EnableCap);
	_out->Write(mData_TextureEnableCap);
	_out->Write(Checkpoint("EnableCapsEnd"));

	_out->Write(Checkpoint("FramebufferObjectsBegin"));
	_out->Write(mData_FrameBufferObjects);
	_out->Write(mData_FrameBufferBindings);
	_out->Write(mData_RenderBufferObjects);
	_out->Write(mData_RenderBufferBindings);
	_out->Write(Checkpoint("FramebufferObjectsEnd"));

	_out->Write(mData_ClipPlaneEquations);

	_out->Write(mData_DrawBuffer);
	_out->Write(mData_ReadBuffer);

	_out->Write(mData_SamplerObjects);
	_out->Write(mData_SamplerBindings);

	_out->Write(mData_VertexAttribEnabled);

	_out->Write(Checkpoint("ContextStateEnd"));
}

// ------------------------------------------------------------------------------------------------
void ContextState::ManualRead(FileLike* _in)
{
	_in->Read(Checkpoint("ContextStateBegin"));

	_in->Read(Checkpoint("TexturesBegin"));
	_in->Read(&mData_TextureObjects);
	_in->Read(&mData_TextureUnits);
	_in->Read(Checkpoint("TexturesEnd"));

	_in->Read(&mData_PixelStoreState);
	_in->Read(&mData_PixelTransferState);

	_in->Read(Checkpoint("BuffersBegin"));
	_in->Read(&mData_BufferObjects);
	_in->Read(&mData_BufferBindings);
	_in->Read(Checkpoint("BuffersEnd"));

	_in->Read(Checkpoint("ShadersBegin"));
	_in->Read(&mData_ShaderObjectsGLSL);
	_in->Read(Checkpoint("ShadersEnd"));

	_in->Read(Checkpoint("ProgramsBegin"));
	_in->Read(&mData_ProgramObjectsGLSL);
	_in->Read(Checkpoint("ProgramsEnd"));

	_in->Read(Checkpoint("ProgramsARBBegin"));
	_in->Read(&mData_ProgramBindingsARB);
	_in->Read(&mData_ProgramObjectsARB);
	_in->Read(Checkpoint("ProgramsARBEnd"));

	_in->Read(Checkpoint("EnableCapsBegin"));
	_in->Read(&mData_EnableCap);
	_in->Read(&mData_TextureEnableCap);
	_in->Read(Checkpoint("EnableCapsEnd"));

	_in->Read(Checkpoint("FramebufferObjectsBegin"));
	_in->Read(&mData_FrameBufferObjects);
	_in->Read(&mData_FrameBufferBindings);
	_in->Read(&mData_RenderBufferObjects);
	_in->Read(&mData_RenderBufferBindings);
	_in->Read(Checkpoint("FramebufferObjectsEnd"));

	_in->Read(&mData_ClipPlaneEquations);

	_in->Read(&mData_DrawBuffer);
	_in->Read(&mData_ReadBuffer);

	_in->Read(&mData_SamplerObjects);
	_in->Read(&mData_SamplerBindings);

	_in->Read(&mData_VertexAttribEnabled);

	_in->Read(Checkpoint("ContextStateEnd"));
}

// ------------------------------------------------------------------------------------------------
void ContextState::ManualPreRestore()
{
	// Make sure no FBO is bound--will be correctly set later in ManualRestore.
	::glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERROR();
	if (mData_DrawBuffer != GL_NONE) {
		::glDrawBuffer(mData_DrawBuffer);
	}

	if (mData_ReadBuffer != GL_NONE) {
		::glReadBuffer(mData_ReadBuffer);
	}

	CHECK_GL_ERROR();
}

// ------------------------------------------------------------------------------------------------
void ContextState::ManualRestore()
{
	CHECK_GL_ERROR();

	// Do enables/disables
	for (auto it = mData_EnableCap.cbegin(); it != mData_EnableCap.cend(); ++it) {
		if (it->second) {
			::glEnable(it->first);
		} else {
			::glDisable(it->first);
		}
		CHECK_GL_ERROR();
	}

	// To restore later
	GLint activeTex = 0;
	::glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTex);

	// Do texture enables/disables. Note: Not really trying to sort these now, probably should consider it.
	for (auto it = mData_TextureEnableCap.cbegin(); it != mData_TextureEnableCap.cend(); ++it) {
		::glActiveTexture(it->first.first);
		if (it->second) {
			::glEnable(it->first.second);
		} else {
			::glDisable(it->first.second);
		}
		CHECK_GL_ERROR();
	}

	// Restore active texture
	::glActiveTexture(activeTex);

	CHECK_GL_ERROR();
}

// ------------------------------------------------------------------------------------------------
void ContextState::glAttachShader(GLuint program, GLuint shader)
{
	auto progIt = mData_ProgramObjectsGLSL.find(program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}

	progIt->second->glAttachShader(program, shader);

	auto shadIt = mData_ShaderObjectsGLSL.find(shader);
	if (shadIt != mData_ShaderObjectsGLSL.end() && shadIt->second != NULL) {
		shadIt->second->OnShaderAttach();
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindTexture(GLenum target, GLuint texture)
{
	if (texture != 0) {
		auto texIt = mData_TextureObjects.find(texture);
		if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
			texIt->second->glBindTexture(target, texture);
		} else {
			mData_TextureObjects[texture] = new GLTexture(target);
		}
	}

	mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)] = texture;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data)
{
	GLenum bindTarget = TexImage2DTargetToBoundTarget(target);

	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, bindTarget)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glCompressedTexImage2D(this, target, level, internalformat, width, height, border, imagesize, data);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid* data)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glCompressedTexImage3D(this, target, level, internalformat, width, height, depth, border, imagesize, data);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteShader(GLuint shader)
{
	if (shader == 0) {
		return;
	}

	auto shadIt = mData_ShaderObjectsGLSL.find(shader);
	if (shadIt != mData_ShaderObjectsGLSL.end() && shadIt->second != NULL) {
		shadIt->second->glDeleteShader(shader);
		if (shadIt->second->GetAttachCount() == 0) {
			// Delete it.
			GLShader* deadShader = shadIt->second;
			mData_ShaderObjectsGLSL.erase(shadIt);
			SafeDelete(deadShader);		
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteTextures(GLsizei n, const GLuint* textures)
{
	// Spec says that textures that are deleted while bound are replaced by the default texture, do that here.
	for (auto it = mData_TextureUnits.begin(); it != mData_TextureUnits.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == textures[i])
			{
				// Texture is bound, unbind it.
				it->second = 0;
			}
		}
	}

	// Then, remove the name.
	for (int i = 0; i < n; ++i) {
		auto texIt = mData_TextureObjects.find(textures[i]);
		if (texIt != mData_TextureObjects.end()) {
			SafeDelete(texIt->second);
			mData_TextureObjects.erase(texIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDisable(GLenum cap)
{
	// More safety checking here would only make this code more brittle.
	switch (cap) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_GEN_Q:
		case GL_TEXTURE_GEN_R:
		case GL_TEXTURE_GEN_S:
		case GL_TEXTURE_GEN_T:
			mData_TextureEnableCap[std::make_pair(mData_glActiveTexture.texture, cap)] = GL_FALSE;
			break;
		
		default:
			mData_EnableCap[cap] = GL_FALSE;
			break;
	};
}

// ------------------------------------------------------------------------------------------------
void ContextState::glEnable(GLenum cap)
{
	// More safety checking here would only make this code more brittle.
	switch (cap) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_3D:
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_GEN_Q:
		case GL_TEXTURE_GEN_R:
		case GL_TEXTURE_GEN_S:
		case GL_TEXTURE_GEN_T:
			mData_TextureEnableCap[std::make_pair(mData_glActiveTexture.texture, cap)] = GL_TRUE;
			break;

		default:
			mData_EnableCap[cap] = GL_TRUE;
			break;
	};
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenBuffersARB(GLsizei n, GLuint *buffers)
{
	for (GLsizei i = 0; i < n; ++i) {
		mData_BufferObjects[buffers[i]] = new GLBuffer;
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenTextures(GLsizei n, GLuint *textures)
{
	for (GLsizei i = 0; i < n; ++i) {
		mData_TextureObjects[textures[i]] = new GLTexture;
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glPixelStoref(GLenum pname, GLfloat param)
{
	mData_PixelStoreState.glPixelStoref(pname, param);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glPixelStorei(GLenum pname, GLint param)
{
	mData_PixelStoreState.glPixelStorei(pname, param);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glPixelTransferf(GLenum pname, GLfloat param)
{
	mData_PixelTransferState.glPixelTransferf(pname, param);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glPixelTransferi(GLenum pname, GLint param)
{
	mData_PixelTransferState.glPixelTransferi(pname, param);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid* string)
{
	auto bindIt = mData_ProgramBindingsARB.find(target);
	if (bindIt == mData_ProgramBindingsARB.end()) {
		return;
	}

	auto progARBIt = mData_ProgramObjectsARB.find(bindIt->second);
	if (progARBIt == mData_ProgramObjectsARB.end() || progARBIt->second == NULL) {
		return;
	}
	
	progARBIt->second->glProgramStringARB(target, format, len, string);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	GLenum bindTarget = TexImage2DTargetToBoundTarget(target);
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, bindTarget)];

	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexImage2D(this, target, level, internalformat, width, height, border, format, type, pixels);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexImage3D(this, target, level, internalFormat, width, height, depth, border, format, type, data);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexParameterf(pname, param);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexParameterfv(pname, params);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexParameteri(pname, param);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, target)];
	auto texIt = mData_TextureObjects.find(textureID);
	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexParameteriv(pname, params);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	GLenum bindTarget = TexImage2DTargetToBoundTarget(target);
	
	auto textureID = mData_TextureUnits[std::make_pair(mData_glActiveTexture.texture, bindTarget)];
	auto texIt = mData_TextureObjects.find(textureID);

	if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
		texIt->second->glTexSubImage2D(this, target, level, xoffset, yoffset, width, height, format, type, pixels);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	auto progIt = mData_ProgramObjectsGLSL.find(program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}

	progIt->second->glBindAttribLocation(program, index, name);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindBuffer(GLenum target, GLuint buffer)
{
	if (!IsValidTarget_glBindBuffer(target)) {
		return;
	}

	if (buffer != 0) {
		auto buffIt = mData_BufferObjects.find(buffer);
		if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
			// TODO: Check and see if this texture is of the same type (or is currently typeless)
			mData_BufferObjects[buffer] = new GLBuffer(target);
		}
	}

	mData_BufferBindings[target] = buffer;	
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	// Check validity
	if (framebuffer != 0) {
		auto fbIt = mData_FrameBufferObjects.find(framebuffer);
		if (fbIt == mData_FrameBufferObjects.end() || fbIt->second == NULL) {
			return;
		}
	}

	// Done this way because of the ability to bind both in one call.
	bool bindRead = (target == GL_READ_FRAMEBUFFER || target == GL_FRAMEBUFFER);
	bool bindDraw = (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER);

	if (bindRead) {
		mData_FrameBufferBindings[GL_READ_FRAMEBUFFER] = framebuffer;
	}

	if (bindDraw) {
		mData_FrameBufferBindings[GL_DRAW_FRAMEBUFFER] = framebuffer;
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	if (texture != 0) {
		auto texIt = mData_TextureObjects.find(texture);
		if (texIt != mData_TextureObjects.end() && texIt->second != NULL) {
			// TODO: Check and see if this texture is of the same type (or is currently typeless)
		} else {
			mData_TextureObjects[texture] = new GLTexture(target);
		}
	}

	mData_TextureUnits[std::make_pair(texunit, target)] = texture;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindProgramARB(GLenum target, GLuint program)
{
	if (!IsValidTarget_glBindProgramARB(target)) {
		return;
	}

	if (program != 0) {
		auto progArbIt = mData_ProgramObjectsARB.find(program);
		if (progArbIt == mData_ProgramObjectsARB.end() || progArbIt->second == NULL) {
			mData_ProgramObjectsARB[program] = new GLProgramARB(this, program, target);
		} else if (!mData_ProgramObjectsARB[program]->CheckAndSetTarget(target)) {
			return;
		}
	}

	mData_ProgramBindingsARB[target] = program;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	// Check validity
	if (renderbuffer != 0) {
		auto rbIt = mData_RenderBufferObjects.find(renderbuffer);
		if (rbIt == mData_RenderBufferObjects.end() || rbIt->second == NULL) {
			return;
		}
	}

	bool bindRenderbuffer = target == GL_RENDERBUFFER;

	if (bindRenderbuffer) {
		mData_RenderBufferBindings[GL_RENDERBUFFER] = renderbuffer;
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBindSampler(GLuint unit, GLuint sampler)
{
	if (sampler != 0) {
		auto samplIt = mData_SamplerObjects.find(sampler);
		if (samplIt == mData_SamplerObjects.end() || samplIt->second == NULL) {
			// Per the spec, needs to have been created with glGenSamplers first
			return;
		}
		samplIt->second->glBindSampler(unit, sampler);
	}

	mData_SamplerBindings[unit] = sampler;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBufferData(GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage)
{
	if (!IsValidTarget_glBufferData(target)) {
		// GL_INVALID_ENUM
		return;
	}

	if (!IsValidBufferUsage(usage)) {
		// GL_INVALID_ENUM
		return;
	}

	if (size < 0) {
		// GL_INVALID_VALUE
		return;
	}

	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	buffIt->second->glBufferData(target, size, data, usage);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
	if (!IsValidTarget_glBufferSubData(target)) {
		// GL_INVALID_ENUM
		return;
	}

	if (size < 0 || offset < 0) {
		// GL_INVALID_VALUE
		return;
	}

	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	buffIt->second->glBufferSubData(target, offset, size, data);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glClipPlane(GLenum plane, const GLdouble* equation)
{
	mData_ClipPlaneEquations[plane] = GLClipPlane(equation);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glCompileShader(GLuint shader)
{
	if (shader == 0) {
		return;
	}

	auto shadIt = mData_ShaderObjectsGLSL.find(shader);
	if (shadIt != mData_ShaderObjectsGLSL.end() && shadIt->second != NULL) {
		shadIt->second->glCompileShader(shader);
	}
}

// ------------------------------------------------------------------------------------------------
GLhandleARB ContextState::glCreateProgramObjectARB(GLhandleARB _retVal)
{
	if (_retVal == 0) {
		return _retVal;
	}

	mData_ProgramObjectsGLSL[_retVal] = new GLProgram(this, _retVal);
	return _retVal;
}

// ------------------------------------------------------------------------------------------------
GLhandleARB ContextState::glCreateShaderObjectARB(GLhandleARB _retVal, GLenum type)
{
	if (_retVal == 0) {
		return _retVal;
	}

	mData_ShaderObjectsGLSL[_retVal] = new GLShader(type, _retVal);

	return _retVal;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteBuffersARB(GLsizei n, const GLuint* buffers)
{
	// First, unbind these buffers if they are bound.
	for (auto it = mData_BufferBindings.begin(); it != mData_BufferBindings.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == buffers[i])
			{
				// Buffer is bound, unbind it.
				it->second = 0;
			}
		}
	}

	// Then, remove the name.
	for (int i = 0; i < n; ++i) {
		if (buffers[n] == 0) {
			continue;
		}

		auto buffIt = mData_BufferObjects.find(buffers[i]);
		if (buffIt != mData_BufferObjects.end()) {
			SafeDelete(buffIt->second);
			mData_BufferObjects.erase(buffIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers)
{
	for (auto it = mData_FrameBufferBindings.begin(); it != mData_FrameBufferBindings.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == framebuffers[i]) {
				it->second = 0;			
			}
		}
	}

	for (int i = 0; i < n; ++i) {
		if (framebuffers[i] == 0) {
			continue;
		}
		
		auto fbIt = mData_FrameBufferObjects.find(framebuffers[i]);
		if (fbIt != mData_FrameBufferObjects.end()) {
			SafeDelete(fbIt->second);
			mData_FrameBufferObjects.erase(fbIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers)
{
	for (auto it = mData_RenderBufferBindings.begin(); it != mData_RenderBufferBindings.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == renderbuffers[i]) {
				it->second = 0;			
			}
		}
	}

	// Now do the attached framebuffer only (per the spec).
	for (auto it = mData_FrameBufferBindings.cbegin(); it != mData_FrameBufferBindings.cend(); ++it) {
		if (it->second == 0) {
			continue;
		}
		
		auto fbIt = mData_FrameBufferObjects.find(it->second);
		if (fbIt == mData_FrameBufferObjects.end() || fbIt->second == NULL) {
			continue;
		}
		
		for (int i = 0; i < n; ++i) {
			fbIt->second->OnDeleteRenderbufferObject(renderbuffers[i]);
		}
	}

	for (int i = 0; i < n; ++i) {
		if (renderbuffers[i] == 0) {
			continue;
		}
		
		auto rbIt = mData_RenderBufferObjects.find(renderbuffers[i]);
		if (rbIt != mData_RenderBufferObjects.end()) {
			SafeDelete(rbIt->second);
			mData_RenderBufferObjects.erase(rbIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteProgramsARB(GLsizei n, const GLuint* programs)
{
	// First, unbind these programs if they are bound.
	for (auto it = mData_ProgramBindingsARB.begin(); it != mData_ProgramBindingsARB.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == programs[i])
			{
				it->second = 0;
			}
		}
	}

	// Then, remove the name.
	for (int i = 0; i < n; ++i) {
		if (programs[n] == 0) {
			continue;
		}

		auto progARBIt = mData_ProgramObjectsARB.find(programs[i]);
		if (progARBIt != mData_ProgramObjectsARB.end()) {
			SafeDelete(progARBIt->second);
			mData_ProgramObjectsARB.erase(progARBIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDeleteSamplers(GLsizei n, const GLuint* samplers)
{
	// First, unbind these samplers if they are bound.
	for (auto it = mData_SamplerBindings.begin(); it != mData_SamplerBindings.end(); ++it) {
		for (int i = 0; i < n; ++i) {
			if (it->second == samplers[i])
			{
				it->second = 0;
			}
		}
	}

	// Then, remove the name.
	for (int i = 0; i < n; ++i) {
		if (samplers[n] == 0) {
			continue;
		}

		auto samplIt = mData_SamplerObjects.find(samplers[i]);
		if (samplIt != mData_SamplerObjects.end()) {
			SafeDelete(samplIt->second);
			mData_SamplerObjects.erase(samplIt);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDetachShader(GLuint program, GLuint shader)
{
	auto progIt = mData_ProgramObjectsGLSL.find(program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}

	bool detached = progIt->second->glDetachShader(program, shader);
	if (detached) {
		// Need to update the attach count in the shader.
		auto shadIt = mData_ShaderObjectsGLSL.find(shader);
		assert(shadIt != mData_ShaderObjectsGLSL.end() && shadIt->second != NULL);
		
		if (shadIt->second->OnShaderDetach()) {
			// Store a pointer, erase it from the table and then delete it.
			GLShader* deadShader = shadIt->second;
			mData_ShaderObjectsGLSL.erase(shadIt);
			SafeDelete(deadShader);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDisableVertexAttribArray(GLuint index)
{
	mData_VertexAttribEnabled[index] = false;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glDrawBuffer(GLenum buffer)
{
	auto fbBindIt = mData_FrameBufferBindings.find(GL_DRAW_FRAMEBUFFER);
	if (fbBindIt != mData_FrameBufferBindings.end() && fbBindIt->second != 0) {
		auto fbIt = mData_FrameBufferObjects.find(fbBindIt->second);
		if (fbIt != mData_FrameBufferObjects.end() && fbIt->second != NULL) {
			fbIt->second->glDrawBuffer(buffer);
			return;
		}
	}

	mData_DrawBuffer = buffer;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glEnableVertexAttribArray(GLuint index)
{
	mData_VertexAttribEnabled[index] = true;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	if (!IsValidTarget_glFlushMappedBufferRange(target)) {
		return;
	}

	if (length < 0 || offset < 0) {
		// GL_INVALID_VALUE
		return;
	}

	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	buffIt->second->glFlushMappedBufferRange(target, offset, length);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	GLenum realTarget = GL_NONE;
	switch (target) {
		case GL_READ_FRAMEBUFFER:
			realTarget = GL_READ_FRAMEBUFFER; 
			break;

		case GL_FRAMEBUFFER:
		case GL_DRAW_FRAMEBUFFER:
			realTarget = GL_DRAW_FRAMEBUFFER;
			break;
		default:
			// glError, returning.
			return;
	};

	auto bindIt = mData_FrameBufferBindings.find(realTarget);
	if (bindIt == mData_FrameBufferBindings.end()) {
		return;
	}

	auto fbIt = mData_FrameBufferObjects.find(bindIt->second);
	if (fbIt == mData_FrameBufferObjects.end() || fbIt->second == NULL) {
		return;
	}

	fbIt->second->glFramebufferRenderbuffer(realTarget, attachment, renderbuffertarget, renderbuffer);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	GLenum realTarget = GL_NONE;
	switch (target) {
		case GL_READ_FRAMEBUFFER:
			realTarget = GL_READ_FRAMEBUFFER; 
			break;

		case GL_FRAMEBUFFER:
		case GL_DRAW_FRAMEBUFFER:
			realTarget = GL_DRAW_FRAMEBUFFER;
			break;
		default:
			// glError, returning.
			return;
	};

	auto bindIt = mData_FrameBufferBindings.find(realTarget);
	if (bindIt == mData_FrameBufferBindings.end()) {
		return;
	}

	auto fbIt = mData_FrameBufferObjects.find(bindIt->second);
	if (fbIt == mData_FrameBufferObjects.end() || fbIt->second == NULL) {
		return;
	}

	fbIt->second->glFramebufferTexture2D(realTarget, attachment, textarget, texture, level);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
	GLenum realTarget = GL_NONE;
	switch (target) {
		case GL_READ_FRAMEBUFFER:
			realTarget = GL_READ_FRAMEBUFFER; 
			break;

		case GL_FRAMEBUFFER:
		case GL_DRAW_FRAMEBUFFER:
			realTarget = GL_DRAW_FRAMEBUFFER;
			break;
		default:
			// glError, returning.
			return;
	};

	auto bindIt = mData_FrameBufferBindings.find(realTarget);
	if (bindIt == mData_FrameBufferBindings.end()) {
		return;
	}

	auto fbIt = mData_FrameBufferObjects.find(bindIt->second);
	if (fbIt == mData_FrameBufferObjects.end() || fbIt->second == NULL) {
		return;
	}

	fbIt->second->glFramebufferTexture3D(realTarget, attachment, textarget, texture, level, layer);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenFramebuffers(GLsizei n, GLuint* ids)
{
	for (int i = 0; i < n; ++i) {
		mData_FrameBufferObjects[ids[i]] = new GLFrameBufferObject(ids[i]);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	for (int i = 0; i < n; ++i) {
		mData_RenderBufferObjects[renderbuffers[i]] = new GLRenderBufferObject(GL_NONE, renderbuffers[i]);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenProgramsARB(GLsizei n, GLuint* programs)
{
	for (int i = 0; i < n; ++i) {
		mData_ProgramObjectsARB[programs[i]] = new GLProgramARB(this, programs[i]);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glGenSamplers(GLsizei n, GLuint* samplers)
{
	for (int i = 0; i < n; ++i) {
		mData_SamplerObjects[samplers[i]] = new GLSampler(samplers[i]);
	}
}

// ------------------------------------------------------------------------------------------------
GLint ContextState::glGetUniformLocation(GLint _retVal, GLuint program, const GLchar* name)
{
	auto progIt = mData_ProgramObjectsGLSL.find(program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		if (_retVal != -1) {
			Once(TraceError(TC("glGetUniformLocation is returning a valid location, but we cannot find the bound program--trace replay is bad.")));
		}
		return _retVal;
	}

	progIt->second->glGetUniformLocation(_retVal, program, name);
	return _retVal;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glLinkProgram(GLuint program)
{
	auto progIt = mData_ProgramObjectsGLSL.find(program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}
	
	progIt->second->glLinkProgram(program);
}

// ------------------------------------------------------------------------------------------------
GLvoid* ContextState::glMapBufferARB(GLvoid* _retVal, GLenum target, GLenum access)
{
	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return NULL;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	return buffIt->second->glMapBuffer(_retVal, target, access);
}

// ------------------------------------------------------------------------------------------------
GLvoid* ContextState::glMapBufferRange(GLvoid* _retVal, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return NULL;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	return buffIt->second->glMapBufferRange(_retVal, target, offset, length, access);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	if (target != GL_RENDERBUFFER) {
		return;
	}

	auto bindIt = mData_RenderBufferBindings.find(GL_RENDERBUFFER);
	if (bindIt == mData_RenderBufferBindings.end() || bindIt->second == 0) {
		return;
	}

	auto rbIt = mData_RenderBufferObjects.find(bindIt->second);
	if (rbIt == mData_RenderBufferObjects.end() || rbIt->second == NULL) {
		return;
	}

	rbIt->second->glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glReadBuffer(GLenum buffer)
{
	auto fbBindIt = mData_FrameBufferBindings.find(GL_DRAW_FRAMEBUFFER);
	if (fbBindIt != mData_FrameBufferBindings.end() && fbBindIt->second != 0) {
		auto fbIt = mData_FrameBufferObjects.find(fbBindIt->second);
		if (fbIt != mData_FrameBufferObjects.end() && fbIt->second != NULL) {
			fbIt->second->glReadBuffer(buffer);
			return;
		}
	}

	mData_ReadBuffer = buffer;
}

// ------------------------------------------------------------------------------------------------
void ContextState::glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	if (sampler != 0) {
		auto samplIt = mData_SamplerObjects.find(sampler);
		if (samplIt == mData_SamplerObjects.end() || samplIt->second == NULL) {
			// Per the spec, needs to have been created with glGenSamplers first
			return;
		}

		samplIt->second->glSamplerParameterf(sampler, pname, param);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* params)
{
	if (sampler != 0) {
		auto samplIt = mData_SamplerObjects.find(sampler);
		if (samplIt == mData_SamplerObjects.end() || samplIt->second == NULL) {
			// Per the spec, needs to have been created with glGenSamplers first
			return;
		}

		samplIt->second->glSamplerParameterfv(sampler, pname, params);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	if (sampler != 0) {
		auto samplIt = mData_SamplerObjects.find(sampler);
		if (samplIt == mData_SamplerObjects.end() || samplIt->second == NULL) {
			// Per the spec, needs to have been created with glGenSamplers first
			return;
		}

		samplIt->second->glSamplerParameteri(sampler, pname, param);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glShaderSource(GLuint shader, GLsizei count, const GLcharARB** string, const GLint* length)
{
	auto shadIt = mData_ShaderObjectsGLSL.find(shader);
	if (shadIt != mData_ShaderObjectsGLSL.end() && shadIt->second != NULL) {
		shadIt->second->glShaderSource(shader, count, string, length);
	}
}

// ------------------------------------------------------------------------------------------------
void ContextState::glUniform1f(GLint location, GLfloat v0)
{
	auto progIt = mData_ProgramObjectsGLSL.find(mData_glUseProgram.program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}
	
	progIt->second->glUniform<1, GLfloat>(location, 1, &v0);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glUniform1i(GLint location, GLint v0)
{
	auto progIt = mData_ProgramObjectsGLSL.find(mData_glUseProgram.program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}
	
	progIt->second->glUniform<1, GLint>(location, 1, &v0);
}

// ------------------------------------------------------------------------------------------------
void ContextState::glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	auto progIt = mData_ProgramObjectsGLSL.find(mData_glUseProgram.program);
	if (progIt == mData_ProgramObjectsGLSL.end() || progIt->second == NULL) {
		return;
	}
	
	progIt->second->glUniform<4, GLfloat>(location, count, value);
}

// ------------------------------------------------------------------------------------------------
GLboolean ContextState::glUnmapBuffer(GLboolean _retVal, GLenum target)
{
	auto bindIt = mData_BufferBindings.find(target);
	if (bindIt == mData_BufferBindings.end() || bindIt->second == 0) { 
		// GL_INVALID_OPERATION
		return GL_FALSE;
	}

	auto buffIt = mData_BufferObjects.find(bindIt->second);
	if (buffIt == mData_BufferObjects.end() || buffIt->second == NULL) {
		TraceError(TC("glTrace Internal error with buffer (id: %d) bound at (target: %d)"), buffIt->first, target);
		assert(0);
	}

	buffIt->second->glUnmapBuffer(target);

	return _retVal;
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_SwapBuffers(HDC hdc)
{

}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindBuffer(GLenum target, GLuint buffer)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayBufferHandle(buffer);
	::glBindBuffer(target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayFrameBufferObjectHandle(framebuffer);
	::glBindFramebuffer(target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayTextureHandle(texture);
	::glBindMultiTextureEXT(texunit, target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindProgramARB(GLenum target, GLuint program)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayProgramARBHandle(program);
	::glBindProgramARB(target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayRenderBuffer(renderbuffer);
	::glBindRenderbuffer(target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindSampler(GLuint unit, GLuint sampler)
{
	GLuint replayHandle = GetReplayTrace()->GetReplaySamplerHandle(sampler);
	::glBindSampler(unit, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glBindTexture(GLenum target, GLuint texture)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayTextureHandle(texture);
	::glBindTexture(target, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glDeleteSamplers(GLsizei n, const GLuint* samplers)
{
	for (int i = 0; i < n; ++i) {
		GLuint replayHandle = GetReplayTrace()->GetReplaySamplerHandle(samplers[i]);
		if (replayHandle != 0) {
			::glDeleteSamplers(1, &replayHandle);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayRenderBuffer(renderbuffer);
	::glFramebufferRenderbuffer(target, attachment, renderbuffertarget, replayHandle);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayRenderBuffer(texture);
	::glFramebufferTexture2D(target, attachment, textarget, replayHandle, level);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
	GLuint replayHandle = GetReplayTrace()->GetReplayRenderBuffer(texture);
	::glFramebufferTexture3D(target, attachment, textarget, replayHandle, level, layer);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glGetQueryObjectivARB(GLuint id, GLenum pname, GLint* params)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glGetQueryObjectuivARB(GLuint id, GLenum pname, GLuint* params)
{
	// TODO
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	GLuint replayHandle = GetReplayTrace()->GetReplaySamplerHandle(sampler);
	::glSamplerParameterf(replayHandle, pname, param);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* params)
{
	GLuint replayHandle = GetReplayTrace()->GetReplaySamplerHandle(sampler);
	::glSamplerParameterfv(replayHandle, pname, params);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	GLuint replayHandle = GetReplayTrace()->GetReplaySamplerHandle(sampler);
	::glSamplerParameteri(replayHandle, pname, param);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glUniform1f(GLint location, GLfloat v0)
{
	GLuint replayLocation = GetReplayTrace()->GetUniformLocation(location);
	::glUniform1f(replayLocation, v0);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glUniform1i(GLint location, GLint v0)
{
	GLuint replayLocation = GetReplayTrace()->GetUniformLocation(location);
	::glUniform1i(replayLocation, v0);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	GLuint replayLocation = GetReplayTrace()->GetUniformLocation(location);
	::glUniform4fv(replayLocation, count, value);
}

// ------------------------------------------------------------------------------------------------
void ManualPlay_glUseProgram(GLuint program)
{
	GLuint replayHandle= GetReplayTrace()->GetReplayProgramGLSLHandle(program);
	::glUseProgram(replayHandle);

	// Let the trace know what program is in use so it can properly return uniform locations.
	GetReplayTrace()->glUseProgram(replayHandle);
}
