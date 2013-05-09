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
#include "filelike.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Checkpoint::Checkpoint(const char* _str)
: mToken(_str)
, mTokenLength(0)
{
	mTokenLength = strlen(mToken) + 1;
}

// ------------------------------------------------------------------------------------------------
void Checkpoint::Write(FileLike* _out) const
{
	_out->Write(mToken, mTokenLength);
}

// ------------------------------------------------------------------------------------------------
void Checkpoint::Read(FileLike* _in) const
{
	if (mTokenLength < 64) {
		char buffer[64];
		_in->Read(buffer, mTokenLength);
		if (strcmp(buffer, mToken) != 0) {
			throw 10;
		}
	} else {
		char* buffer = new char[mTokenLength];
		_in->Read(buffer, mTokenLength);
		if (strcmp(buffer, mToken) != 0) {
			SafeDelete(buffer);
			throw 10;
		}
		SafeDelete(buffer);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
FileLike::FileLike(FILE* fp)
: mMode(FileLike::File)
, mFile(fp)
, mMessageStream(NULL)
{

}

// ------------------------------------------------------------------------------------------------
FileLike::FileLike(MessageStream *_msgStream)
: mMode(FileLike::Socket)
, mFile(NULL)
, mMessageStream(_msgStream)
{

}

// ------------------------------------------------------------------------------------------------
size_t FileLike::AllocatePacketId()
{
	switch (mMode) {
	case FileLike::File:	return 0;
	case FileLike::Socket:	return mMessageStream->AllocatePacketId();
	default: assert(!"Invalid mode in FileLike::AllocatePacketId"); break;
	}
	return (size_t)-1;
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(bool* _val)
{
	size_t ReadThis = 0;
	Read(&ReadThis);
	(*_val) = (ReadThis != 0);
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(char* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(unsigned char* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(short* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(unsigned short* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(int* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(unsigned int* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(float* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(double* _val)
{
	ReadRaw(_val, sizeof(*_val));
}

// ------------------------------------------------------------------------------------------------
size_t FileLike::Read(void* _bytes, size_t _len)
{
	size_t bytesInStream = 0;
	Read(&bytesInStream);
	
	if (bytesInStream > 0) {
		assert(_len >= bytesInStream);
		ReadRaw(_bytes, min(_len, bytesInStream));
	}

	return min(_len, bytesInStream);
}

// ------------------------------------------------------------------------------------------------
void FileLike::Read(void** _bytes, size_t* _outLen)
{
	Read(_outLen);
	if ((*_outLen) > 0) {
		(*_bytes) = malloc(*_outLen);
		assert(*_bytes);

		ReadRaw((*_bytes), (*_outLen));
	}
}


// ------------------------------------------------------------------------------------------------
void FileLike::ReadRaw(void* _bytes, size_t _len)
{
	assert((mFile != 0) ^ (mMessageStream != 0));

	switch(mMode) {
		case FileLike::File:	
		{
			if (1 != fread(_bytes, _len, 1, mFile)) { 
				throw 10; 
			} 
			break;
		}

		case FileLike::Socket:	
		{
			mMessageStream->BlockingRecv(_bytes, _len);
			break;
		}

		default: 
			assert(!"Invalid mode in FileLike::Read"); break;
	}
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(bool _val)
{
	size_t WriteThis = _val ? 1 : 0;
	Write(WriteThis);
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(char _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(unsigned char _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(short _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(unsigned short _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(int _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(unsigned int _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(float _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(double _val)
{
	WriteRaw(&_val, sizeof(_val));
}

// ------------------------------------------------------------------------------------------------
void FileLike::Write(const void* _bytes, size_t _len)
{
	Write(_len);
	if (_len) {
		WriteRaw(_bytes, _len);
	}
}

// ------------------------------------------------------------------------------------------------
void FileLike::WriteRaw(const void* _bytes, size_t _len)
{
	assert((mFile != 0) ^ (mMessageStream != 0));
	switch (mMode) {
	case FileLike::File:	if (1 != fwrite(_bytes, _len, 1, mFile)) { throw 10; } break;
	case FileLike::Socket:	mMessageStream->Send(_bytes, _len); break;
	default: assert(!"Invalid mode in FileLike::Read"); break;
	}
}
