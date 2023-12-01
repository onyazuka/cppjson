#pragma once
#include "Socket.hpp"

namespace inet {

	class TcpNonblockingSocket : public ISocket {
	public:
		TcpNonblockingSocket(int _fd);
		~TcpNonblockingSocket();
		int init() const override;
		std::pair<ssize_t, std::shared_ptr<ISocket>> accept() const override;
		ssize_t read(InputSocketBuffer& buf) const override;
		ssize_t write(OutputSocketBuffer& buf) const override;
		//int shutdown(int flags);
		int close() override;
		inline int fd() const override {
			return _fd;
		}
		std::string strerr() const override;
	private:

		enum class Error {
			NoError,
			AcceptNonBlocking,
			AcceptFail,
			ReadClientClose,
			ReadError,
			WriteClientClose,
			WriteError
		};

		int _fd = -1;
		mutable std::pair<Error, int> lastErr = { Error::NoError, 0 };
	};

}