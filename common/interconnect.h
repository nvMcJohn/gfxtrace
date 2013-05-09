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

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

static const char* kPort = "34199";
struct SSerializeDataPacket;

class FileLike;
class SimpleBuffer;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class NetworkError : public std::exception
{
public:
	NetworkError(int _errorNum);

	virtual const char* what() const { return "Networking Error."; }
	int GetErrorNum() const { return mErrorNum; }
	bool IsConnectionReset() const { return mErrorNum == WSAECONNRESET; }
private:
	int mErrorNum;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class MessageStream
{
public:
	MessageStream(bool _isHost, const char* _address, const char* _port);
	~MessageStream();

	inline size_t AllocatePacketId() { return mNextPacketId++; }

	// Sends always succeed--or they raise an exception.
	inline void Send(const void* _bytes, size_t _len)		{ BufferedSend(_bytes, _len); }

	template <typename T> 
	inline void OptionalSend(const T& _t)					{ BufferedSend((const void *)&_t, sizeof(_t), true); }

	inline bool Recv(void* _out, size_t _len)
	{
		size_t totalDataRead = 0;
		do {
			int dataRead = recv(mSocket, ((char*)_out) + totalDataRead, _len - totalDataRead, 0);
			if (dataRead == SOCKET_ERROR) {
				int errorNum = WSAGetLastError(); 
				if (errorNum == WSAEWOULDBLOCK) {
					if (totalDataRead == 0) { 
						return false;
					} else {
						// I don't do partial reads--once I start receiving I wait for everything.
						Sleep(1);
					}
				// I've split these into two blocks because one of them is expected and the other isn't.
				} else if (errorNum == WSAECONNRESET) {
					// The remote client disconnected, probably not an issue.
					throw NetworkError(errorNum);
				} else {
					// Some other wonky network error--place a breakpoint here.
					throw NetworkError(errorNum);
				}
			} else {
				totalDataRead += dataRead;
			}
		} while (totalDataRead < _len);

		return true;
	}

	inline void BlockingRecv(void* _outBuffer, size_t _len)
	{
		while (!Recv(_outBuffer, _len)) {
			Sleep(1);
		}
	}

	void FlushSendBuffer(bool _optional=false);

private:
	SOCKET mSocket;
	struct addrinfo *mHostAddressInfo;
	size_t mNextPacketId;
	SimpleBuffer* mSendBuffer;

	// Used if someone asks for a receive of a small string.
	char mSmallBuffer[64];

	std::string mAddress;
	std::string mPort;
	bool mHost;

	void SetupSocket();

	// Note that Host is the application being instrumented
	void SetupHostSocket();
	// Server is eztrace.exe--but note that eztrace actually connects to the host (to avoid having to 
	// communicate anything to the host ahead of time).
	void SetupServerSocket();

	void Handshake();

	void BufferedSend(const void* _bytes, size_t _size, bool _optional=false);
	void ReallySend(const void* _bytes, size_t _size, bool _optional=false);
};

extern MessageStream* gMessageStream;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class SimpleBuffer
{
public:
	SimpleBuffer(size_t _bufferSize);
	~SimpleBuffer();

	bool AddBytes(const void* _bytes, size_t _size);
	void EmptyBuffer() { mEnd = 0; }
	bool WouldOverflow(size_t _requestedSize) const;
	const void* GetBytes(size_t* _outByteCount) const 
	{ 
		(*_outByteCount) = mEnd; 
		return mBuffer; 
	}

private:
	void* mBuffer;
	size_t mEnd;
	size_t mSize;
};

// ------------------------------------------------------------------------------------------------
enum EnumRemoteCommand 
{
	ERC_None, 
	ERC_Capture,
	ERC_Terminate,
};

// ------------------------------------------------------------------------------------------------
struct RemoteCommand
{
	EnumRemoteCommand mRemoteCommandType;

	RemoteCommand(EnumRemoteCommand _type=ERC_None) : mRemoteCommandType(_type) { }

	void Read(FileLike* _fileLike);
	void Write(FileLike* _fileLike) const;
};

// ------------------------------------------------------------------------------------------------
struct RC_Capture : public RemoteCommand { RC_Capture() : RemoteCommand(ERC_Capture) { } };
struct RC_Terminate : public RemoteCommand { RC_Terminate() : RemoteCommand(ERC_Terminate) { } };
