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

class ContextState;
class GLTrace;

enum ShaderBuildStatus
{
	SBS_Unbuilt = 0,
	SBS_Success,
	SBS_Failure 
};

enum UniformType
{
	UT_Typeless = 0,
	UT_Float,
	UT_Int
};

const int kEntriesPerVector = 4;

// ------------------------------------------------------------------------------------------------
class GLUniformVector
{
public:
	GLUniformVector();
	~GLUniformVector();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	template <int Dimensions, typename Type>
	void Set(const Type* _data)
	{
		mDimensions = Dimensions;

		SetType<Type>(_data);
		CompileTimeAssert(Dimensions <= kEntriesPerVector);
		for (int i = 0; i < Dimensions; ++i) {
			SetSingle<Type>(i, _data[i]);
		}		
	}

	// Should never be used, verify that with compile time asserts.
	template <typename Type>	void SetSingle(int _elem, Type _data)		{ CompileTimeAssert(0); }
	template <typename Type>	void SetType(const Type* _unused)			{ CompileTimeAssert(0); }

	template<>					void SetSingle(int _elem, GLfloat _data)	{ mFloat.data[_elem] = _data; }
	template<>					void SetSingle(int _elem, GLint _data)		{ mInt.data[_elem] = _data;	}

	// Asserts aren't strictly required, but indicates a probable bug.
	template<>					void SetType(const GLfloat* _unused)		{ assert(mType == UT_Typeless || mType == UT_Float); mType = UT_Float; }
	template<>					void SetType(const GLint* _unused)			{ assert(mType == UT_Typeless || mType == UT_Int); mType = UT_Int; }

	void Create(GLint _replayDest) const;

	UniformType GetType() const { return mType; }

private:
	UniformType mType;
	int mDimensions;

	union {
		struct { GLfloat data[kEntriesPerVector]; } mFloat;
		struct { GLint   data[kEntriesPerVector]; } mInt;
	};
};

// ------------------------------------------------------------------------------------------------
class GLShader
{
public:
	GLShader(GLenum _type=GL_NONE, GLuint _shader=0);
	~GLShader();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void OnShaderAttach() 
	{ 
		++mAttachCount; 
	}

	bool OnShaderDetach() 
	{ 
		assert(mAttachCount > 0);
		--mAttachCount; 
		if (mAttachCount == 0 && mMarkedForDelete) { 
			return true; 
		} 
		return false;
	}

	int GetAttachCount() const { return mAttachCount; }

    void glCompileShader(GLuint shader);
	void glDeleteShader(GLuint shader); 
	void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);

	GLuint Create(const GLTrace* _trace) const;

private:
	GLenum mType;
	GLuint mShader;
	ShaderBuildStatus mShaderCompileStatus; 
	int mAttachCount; 
	bool mMarkedForDelete;

	std::vector<std::string> mShaderSourceStrings;
};

// ------------------------------------------------------------------------------------------------
// GLSL Programs. (TODO: Rename)
class GLProgram
{
public:
	GLProgram(ContextState* _ctxState = NULL, GLuint program = 0);
	~GLProgram();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glAttachShader(GLuint program, GLuint shader);
	void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);
	bool glDetachShader(GLuint program, GLuint shader);
	GLint glGetUniformLocation(GLint _retVal, GLuint program, const GLchar* name);
	void glLinkProgram(GLuint program);

	template <int Dimensions, typename Type>
	void glUniform(GLint _location, GLint _count, const Type* _vData)
	{
		for (std::vector<GLUniformVector>::size_type i = 0; signed(i) < _count; ++i) {
			auto dstLocation = _location + i;
			auto srcLocation = i;

			if (dstLocation >= mUniforms.size()) {
				// Allowed by the GL spec, don't crash.
				break;
			}

			
			mUniforms[dstLocation].Set<Dimensions, Type>(_vData + srcLocation * Dimensions);
		}
	}

	GLuint Create(const GLTrace* _trace, std::map<GLint, GLint>* _outUniformMapping) const;

private:
	GLuint mProgram;
	ContextState* mCtxState;

	ShaderBuildStatus mProgramLinkStatus;

	std::vector<GLuint> mAttachedShaders;
	std::vector<GLUniformVector> mUniforms;

	std::map<std::string, GLuint> mAttribBinds;
	std::map<std::string, GLuint> mPendingAttribBinds;

	std::map<std::string, GLint> mUniformLocations;
	
	void InitializeUniformState(ContextState* _ctxState);
};

// ------------------------------------------------------------------------------------------------
class GLProgramARB
{
public:
	GLProgramARB(ContextState* _ctxState = NULL, GLuint program = 0, GLenum target = 0);
	~GLProgramARB();

	void Write(FileLike* _out) const;
	void Read(FileLike* _in);

	void glProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid* string);

	bool CheckAndSetTarget(GLenum target);

	GLuint Create(const GLTrace* _trace) const;
private:
	GLuint mProgram;
	GLenum mTarget;

	std::string mProgramString;
};
