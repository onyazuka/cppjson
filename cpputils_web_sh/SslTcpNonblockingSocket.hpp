#pragma once
#include "Socket.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace inet {

	class SslTcpNonblockingSocket : public ISocket {
	public:
		SslTcpNonblockingSocket(std::shared_ptr<ISocket> psock, SSL* ssl);
		~SslTcpNonblockingSocket();
		SslTcpNonblockingSocket(const SslTcpNonblockingSocket&) = delete;
		SslTcpNonblockingSocket& operator=(const SslTcpNonblockingSocket&) = delete;
		int init() const override;
		std::pair<ssize_t, std::shared_ptr<ISocket>> accept() const override;
		ssize_t read(InputSocketBuffer& buf) const override;
		ssize_t write(OutputSocketBuffer& buf) const override;
		//int shutdown(int flags);
		int close() override;
		inline int fd() const override {
			return psock->fd();
		}
		std::string strerr() const override;

		// when using, user should correctly initialize static ssl ctx with paths to certificate and private key
		struct SslCtx {
			SslCtx(const std::string& CertPath, const std::string& PrivKeyPath);
			~SslCtx();
			mutable SSL_CTX* sslCtx = nullptr;
		};

		inline SSL* getSsl() const { return ssl; }
	private:

		enum class Error {
			NoError,
			AcceptUnderlyingSocket,
			AcceptSslNew,
			AcceptSetFd,
			AcceptFail,
			ReadClientClose,
			ReadError,
			WriteClientClose,
			WriteError
		};

		static SslCtx ctx;
		std::shared_ptr<ISocket> psock;
		mutable SSL* ssl = nullptr;
		static char errBuf[256];
		mutable std::pair<Error, int> lastErr = { Error::NoError, 0 };
		bool sslAcceptFinished = false;
	};

}