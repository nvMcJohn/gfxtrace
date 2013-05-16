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
#include "extensions.h"

#define STR(_s) XSTR(_s)
#define XSTR(_s) #_s

GLuint CompileShader(GLenum _shaderType, const char* _programText)
{
	GLuint retVal = glCreateShader(_shaderType);

	if (!(retVal)) {
		throw 11;
	}

	glShaderSource(retVal, 1, &_programText, NULL);
	glCompileShader(retVal);

	GLint result = GL_FALSE;
	GLint infoLogLength = 0;

	glGetShaderiv(retVal, GL_COMPILE_STATUS, &result);
    glGetShaderiv(retVal, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<GLchar> errorMessage(infoLogLength);
    glGetShaderInfoLog(retVal, infoLogLength, NULL, &errorMessage[0]);

	return retVal;
}

GLuint LinkProgram(GLuint _vs, GLuint _fs, const char* _fragDataLocation)
{
	GLuint retVal = glCreateProgram();
	glAttachShader(retVal, _vs);
	glAttachShader(retVal, _fs);
	glBindFragDataLocation(retVal, 0, _fragDataLocation);
	glLinkProgram(retVal);

	GLint result = GL_FALSE;
	GLint infoLogLength = 0;

	glGetProgramiv(retVal, GL_LINK_STATUS, &result);
    glGetProgramiv(retVal, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<GLchar> errorMessage( max(infoLogLength, 1) );
    glGetProgramInfoLog(retVal, infoLogLength, NULL, &errorMessage[0]);

	return retVal;
}

#undef REQUIRE_GL_EXT
#define REQUIRE_GL_EXT(_spec, _name) _spec _name;
#include "extensions.gl"
#undef REQUIRE_GL_EXT

// Returns true if all extensions resolve, false otherwise.
bool ResolveExtensions()
{
	bool allResolved = true;
	
    #undef REQUIRE_GL_EXT
	#define REQUIRE_GL_EXT(_spec, _name) \
		_name = (_spec) wglGetProcAddress(STR(_name)); \
		if (_name == NULL) { \
			LogError(TC("Unable to find extension %s"), STR(_name)); \
		} \
		allResolved = allResolved && _name != NULL;
	#include "extensions.gl"
	#undef REQUIRE_GL_EXT
	
	return allResolved;
}
