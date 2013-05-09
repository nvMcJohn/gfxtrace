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

class Checkpoint;
class FileLike;

#include <map>
#include <vector>

// For creating checkpoints (consistency checks) in the various streams we're interacting with.
class Checkpoint
{
public:
	Checkpoint(const char* _str);
	void Write(FileLike* _out) const;
	void Read(FileLike* _in) const;

private:
	const char* mToken;
	size_t mTokenLength;
};

// An interface for interacting with sockets, files, and memory streams with a file-like interface.
// This is a simple file-like interface--it doesn't support rewinding or anything fancy, just fifo 
// reads and writes.
class FileLike
{
public:
	FileLike(FILE* fp);
	FileLike(MessageStream *_msgStream /* TODO: Pass in callback here */); 

	size_t AllocatePacketId();

	void Read(bool* _val);
	void Read(char* _val);
	void Read(unsigned char* _val);

	void Read(short* _val);
	void Read(unsigned short* _val);

	void Read(int* _val);
	void Read(unsigned int* _val);

	void Read(float* _val);
	void Read(double* _val);

	size_t Read(void* _bytes, size_t _len);

	// Reads length from the stream, and if >0 then mallocs a chunk of memory to read into--returns through _bytes.
	void Read(void** _bytes, size_t* _outLen);

	// Normally, Read expects the size to live in the stream prefixing the data to be read.
	// With ReadRaw, no size is expected first, and the bytes are directly read.
	void ReadRaw(void* _bytes, size_t _len);

	void Read(const Checkpoint& _checkpoint) { _checkpoint.Read(this); }

	template <typename T>
	void Read(T* _val) 
	{
		_val->Read(this);
	}

	template <typename T1, typename T2>
	void Read(std::map<T1, T2>* _val)
	{
		size_t mapSize = 0; 
		Read(&mapSize);
		for (size_t u = 0; u < mapSize; ++u) {
			T1 _t1 = T1();
			T2 _t2 = T2();

			Read(&_t1);
			Read(&_t2);
			(*_val)[_t1] = _t2;
		}
	}

	template <typename T1, typename T2>
	void Read(std::map<T1, T2*>* _val)
	{
		size_t mapSize = 0; 
		Read(&mapSize);
		for (size_t u = 0; u < mapSize; ++u) {
			T1 _t1 = T1();
			Read(&_t1);

			GLuint hasT2Data = 0;
			Read(&hasT2Data);

			T2* _t2 = NULL;
			if (hasT2Data != 0) {
				_t2 = new T2;
				Read(_t2);
			}

			(*_val)[_t1] = _t2;
		}
	}

	template <typename T1, typename T2>
	void Read(std::pair<T1, T2>* _val)
	{
		Read(&_val->first);
		Read(&_val->second);
	}

	template <typename T>
	void Read(std::vector<T>* _val)
	{
		size_t vecSize = 0;
		Read(&vecSize);
		(*_val).resize(vecSize);
		for (size_t i = 0; i < vecSize; ++i) {
			Read(&((*_val)[i]));
		}
	}

	void Read(std::string* _val)
	{
		std::string::size_type strSize = 0;
		Read(&strSize);
		(*_val).resize(strSize);
		if (strSize > 0) {
			ReadRaw(&((*_val)[0]), strSize);
		}
	}


	void Write(bool _val);
	void Write(char _val);
	void Write(unsigned char _val);

	void Write(short _val);
	void Write(unsigned short _val);

	void Write(int _val);
	void Write(unsigned int _val);

	void Write(float _val);
	void Write(double _val);

	void Write(const void* _bytes, size_t _len);

	// Normally, Write outputs the _len to the stream first--with WriteRaw the bytes are simply written, 
	// no size parameter first.
	void WriteRaw(const void* _bytes, size_t _len);

	void Write(const Checkpoint& _checkpoint) { _checkpoint.Write(this); }

	template <typename T>
	void Write(const T& _val) 
	{
		_val.Write(this);
	}

	template <typename T1, typename T2>
	void Write(const std::map<T1, T2>& _val)
	{
		size_t mapSize = _val.size();
		Write(mapSize);
		for (auto it = _val.cbegin(); it != _val.cend(); ++it) {
			Write(it->first);
			Write(it->second);
		}
	}

	template <typename T1, typename T2>
	void Write(const std::map<T1, T2*>& _val)
	{
		size_t mapSize = _val.size();
		Write(mapSize);
		for (auto it = _val.cbegin(); it != _val.cend(); ++it) {
			Write(it->first);
			if (it->second != NULL) {
				Write(GLuint(1));
				Write(*(it->second));
			} else {
				Write(GLuint(0));
			}
		}
	}

	template <typename T1, typename T2>
	void Write(const std::pair<T1, T2>& _val)
	{
		Write(_val.first);
		Write(_val.second);
	}

	template <typename T>
	void Write(const std::vector<T>& _val)
	{
		size_t vecSize = _val.size();
		Write(vecSize);
		for (size_t i = 0; i < _val.size(); ++i) {
			Write(_val[i]);
		}
	}
	
	void Write(const std::string& _val)
	{
		Write(_val.size());
		if (_val.size() > 0) {
			WriteRaw(&_val[0], _val.size());
		}
	}

private:
	enum { File, Socket } mMode;
	FILE* mFile;
	MessageStream* mMessageStream;
};

