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
#include "interconnect.h"

#include "common/filelike.h"
#include "common/functionhooks.gen.h"


const size_t kSendBufferSize = 1024 * 1024;

MessageStream* gMessageStream = NULL;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
NetworkError::NetworkError(int _errorNum)
: mErrorNum(_errorNum) 
{ }

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
MessageStream::MessageStream(bool _isHost, const char* _address, const char* _port)
: mHost(_isHost)
, mSocket(INVALID_SOCKET)
, mHostAddressInfo(NULL)
, mNextPacketId(0)
, mSendBuffer(NULL)
, mAddress(_address)
, mPort(_port)
{
	SetupSocket();
}

// ------------------------------------------------------------------------------------------------
MessageStream::~MessageStream()
{
	if (mSendBuffer) { 
		// Try to get our data out.
		FlushSendBuffer(true);
		SafeDelete(mSendBuffer);
	}

	SafeDelete(mHostAddressInfo);

	WSACleanup();

}

// ------------------------------------------------------------------------------------------------
void MessageStream::SetupSocket()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		throw 1;
	}

	if (mHost) {
		SetupHostSocket();
	} else {
		SetupClientSocket();
	}
}

// ------------------------------------------------------------------------------------------------
void MessageStream::SetupHostSocket()
{
	int hr = 0;
	struct addrinfo hostAddrInfo = { 0 };

	hostAddrInfo.ai_family = AF_INET;
    hostAddrInfo.ai_socktype = SOCK_STREAM;
    hostAddrInfo.ai_protocol = IPPROTO_TCP;
    hostAddrInfo.ai_flags = AI_PASSIVE;

	hr = getaddrinfo(NULL, mPort.c_str(), &hostAddrInfo, &mHostAddressInfo);
	if (hr != 0) {
        LogVerbose(TC("Host: Failed getaddrinfo"));
		throw 1;
	}

	SOCKET listenSocket = socket(mHostAddressInfo->ai_family, mHostAddressInfo->ai_socktype, mHostAddressInfo->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		// TODO: Figure out errors
        LogVerbose(TC("Host: Failed creating a listen socket"));
		SafeDelete(mHostAddressInfo);
		throw 2;
	}

	hr = bind(listenSocket, mHostAddressInfo->ai_addr, (int)mHostAddressInfo->ai_addrlen);
	if (hr == SOCKET_ERROR) {
        LogVerbose(TC("Host: Failed binding socket"));
		SafeDelete(mHostAddressInfo);
		closesocket(listenSocket);
		throw 3;
	}

	// Done with this.
	SafeDelete(mHostAddressInfo);

	hr = listen(listenSocket, 1);
	if (hr == SOCKET_ERROR) {
        LogVerbose(TC("Host: Failed listening on socket"));
		closesocket(listenSocket);
		throw 4;
	}

	// Fo reals.
	mSocket = accept(listenSocket, NULL, NULL);
	closesocket(listenSocket);

	if (mSocket == INVALID_SOCKET) {
        LogVerbose(TC("Host: Failed accepting socket connection"));
		throw 5;
	}

	Handshake();

	// mSendBuffer = new SimpleBuffer(kSendBufferSize);
}

// ------------------------------------------------------------------------------------------------
void MessageStream::SetupClientSocket()
{
	int hr = 0;
	struct addrinfo hostAddrInfo = { 0 },
		            *currentAttempt = NULL;

	hostAddrInfo.ai_family = AF_UNSPEC;
    hostAddrInfo.ai_socktype = SOCK_STREAM;
    hostAddrInfo.ai_protocol = IPPROTO_TCP;

	hr = getaddrinfo(mAddress.c_str(), mPort.c_str(), &hostAddrInfo, &mHostAddressInfo);
	if (hr != 0) {
        LogVerbose(TC("Client: Failed getaddrinfo"));
		throw 1;
	}

	for (currentAttempt = mHostAddressInfo; currentAttempt != NULL; currentAttempt = currentAttempt->ai_next) {
		mSocket = socket(currentAttempt->ai_family, currentAttempt->ai_socktype, currentAttempt->ai_protocol);
		
		hr = connect(mSocket, currentAttempt->ai_addr, currentAttempt->ai_addrlen);
		if (hr == SOCKET_ERROR) {
            LogVerbose(TC("Client: Failed connect. Possibly non-fatal."));
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	SafeDelete(mHostAddressInfo);

	if (mSocket == INVALID_SOCKET) {
        LogVerbose(TC("Client: Couldn't find any connections. We're gonna crash."));
		throw 2;
    }

	Handshake();
}

// ------------------------------------------------------------------------------------------------
void MessageStream::Handshake()
{
	FileLike fileLike(this);
	Checkpoint syn("It's a trap!");
	Checkpoint ack(" - Admiral Ackbar");

	if (mHost) {
		fileLike.Write(syn);
		fileLike.Read(&ack);
	} else {
		fileLike.Read(&syn);
		fileLike.Write(ack);
	}

	// Turn on non-blocking modes for sockets now.
	u_long asyncMode = 1;
	ioctlsocket(mSocket, FIONBIO, &asyncMode);
}

// ------------------------------------------------------------------------------------------------
void MessageStream::FlushSendBuffer(bool _optional)
{
	size_t bufferedByteSize = 0;
	const void* bufferBytes = mSendBuffer->GetBytes(&bufferedByteSize);
	if (bufferedByteSize > 0) {
		ReallySend(bufferBytes, bufferedByteSize, _optional);
		mSendBuffer->EmptyBuffer();
	}
}

// ------------------------------------------------------------------------------------------------
void MessageStream::BufferedSend(const void* _bytes, size_t _size, bool _optional)
{
	if (!mSendBuffer) { 
		ReallySend(_bytes, _size, _optional);
		return;
	}

	if (!mSendBuffer->WouldOverflow(_size)) {
		mSendBuffer->AddBytes(_bytes, _size);
	} else { 
		// Time to flush the cache.
 		FlushSendBuffer(false);

		// Check to see if the packet is larger than the send buffer 
		if (mSendBuffer->WouldOverflow(_size)) { 
			ReallySend(_bytes, _size, _optional); 
		} else { 
			mSendBuffer->AddBytes(_bytes, _size);
		}
	} 
}

// ------------------------------------------------------------------------------------------------
void MessageStream::ReallySend(const void* _bytes, size_t _size, bool _optional)
{
	assert(_size > 0);

	size_t bytesSent = 0;
	do {
		int sentThisTime = send(mSocket, (const char*)_bytes + bytesSent, _size - bytesSent, 0);
		if (sentThisTime == SOCKET_ERROR) {
			int socketError = WSAGetLastError();
			if (socketError == WSAEWOULDBLOCK) {
				// Try again. Don't sleep, because that nukes performance from orbit.
				continue;
			}
			
			if (!_optional) { 
				throw 7;
			} 
		}
		if (sentThisTime == 0) {
			if (!_optional) {
				throw 6;
			}
			break;
		}

		bytesSent += sentThisTime;

	} while (bytesSent < _size);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SimpleBuffer::SimpleBuffer(size_t _bufferSize)
: mBuffer(NULL)
, mEnd(0)
, mSize(_bufferSize)
{
	mBuffer = (unsigned char *)malloc(mSize);
	if (!mBuffer) {
		throw 8;
	}
}

// ------------------------------------------------------------------------------------------------
SimpleBuffer::~SimpleBuffer()
{
	free(mBuffer);
	mBuffer = NULL;
}

// ------------------------------------------------------------------------------------------------
bool SimpleBuffer::AddBytes(const void* _bytes, size_t _size)
{
	if (WouldOverflow(_size)) { 
		return false;
	}

	memcpy((unsigned char*)mBuffer + mEnd, _bytes, _size);
	mEnd += _size;

	return true;
}

// ------------------------------------------------------------------------------------------------
bool SimpleBuffer::WouldOverflow(size_t _requestedSize) const
{
	return mEnd + _requestedSize > mSize;
}

// ------------------------------------------------------------------------------------------------
void RemoteCommand::Read(FileLike* _fileLike)
{
	unsigned int myCommand = 0;
	_fileLike->Read(&myCommand);
	mRemoteCommandType = (EnumRemoteCommand)myCommand;
}

// ------------------------------------------------------------------------------------------------
void RemoteCommand::Write(FileLike* _fileLike) const
{
	_fileLike->Write((unsigned int)mRemoteCommandType);

}
