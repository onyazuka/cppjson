#include "TcpNonblockingSocket.hpp"
#include <cassert>
#include <string.h>
#include <fcntl.h>
#include <format>
#include <iostream>

using namespace inet;

TcpNonblockingSocket::TcpNonblockingSocket(int __fd)
	: _fd{ __fd }
{
	//assert(_fd >= 0);
}

TcpNonblockingSocket::~TcpNonblockingSocket() {
	//close();
}

int TcpNonblockingSocket::init() const {
	return 0;
}

std::pair<ssize_t, std::shared_ptr<ISocket>> TcpNonblockingSocket::accept() const {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFd = ::accept(_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientFd >= 0) {
		auto res = std::shared_ptr<ISocket>(new TcpNonblockingSocket(clientFd));
		if (fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK) < 0) {
			lastErr = { Error::AcceptNonBlocking, errno };
			return { -EWOULDBLOCK, res };
		}
		return { 0, res };
	}
	else {
		//if (errno != EAGAIN) {
		lastErr = { Error::AcceptFail, errno };
		//}
		return { -errno, nullptr };
	}
}

ssize_t TcpNonblockingSocket::read(InputSocketBuffer& sockBuf) const {
	size_t nbytes = 0;
	for (;;) {
		ssize_t n = sockBuf.read(&::read, _fd);
		//Log.debug(std::format("n = {}, and errno is {}", n, errno));
		if (n <= 0) {
			if (n == 0) {
				lastErr = { Error::ReadClientClose, errno };
				return 0;
			}
			else if (errno == EAGAIN)
			{
				//Log.debug(std::format("EAGAIN on socket {}", _fd));
				return nbytes ? nbytes : -EAGAIN;
			}
			else {
				// n < 0 - error
				lastErr = { Error::ReadError, errno };
				return -errno;
			}
		}
		else {
			nbytes += n;

			//break;
		}
	}
	return nbytes;
}

ssize_t TcpNonblockingSocket::write(OutputSocketBuffer& sockBuf) const {
	size_t nbytes = 0;
	for (;;) {
		ssize_t n = sockBuf.write(&::write, _fd);
		if (n <= 0) {
			if (n == 0) {
				lastErr = { Error::WriteClientClose, errno };
				return 0;
			}
			else if (errno == EAGAIN) {
				// can't write into socket - sleep and try again
				//Log.debug(std::format("Couldn't write to socket {} - EAGAIN", _fd));
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
				//continue;
				return nbytes ? nbytes : -EAGAIN;
			}
			else {
				lastErr = { Error::WriteError, errno };
				return -errno;
			}
		}
		else if (!sockBuf.finished()) {
			//Log.debug(std::format("Writtten only {} from {} bytes to {}, waiting and trying to write the rest", n, buf.size(), _fd));
			nbytes += n;
			continue;
		}
		else {
			nbytes += n;
			return nbytes;
		}
	}
	return nbytes;
}

int TcpNonblockingSocket::close() {
	int res = ::close(_fd);
	//_fd = -1;
	return res;
}

std::string TcpNonblockingSocket::strerr() const {
	switch (lastErr.first) {
	case Error::AcceptNonBlocking:
		return "couldn't set client socket as non-blocking";
	case Error::AcceptFail:
		return std::format("failed to accept client connection: {}", strerror(lastErr.second));
	case Error::ReadClientClose:
		return std::format("error reading from socket {}: client has closed connection", _fd);
	case Error::ReadError:
		return std::format("error reading from socket {}: {}", _fd, strerror(lastErr.second));
	case Error::WriteClientClose:
		return std::format("error writing to socket {}: client has closed connection", _fd);
	case Error::WriteError:
		return std::format("error writing to socket {}: {}", _fd, strerror(lastErr.second));
	default:
		return "No error";
	}
}