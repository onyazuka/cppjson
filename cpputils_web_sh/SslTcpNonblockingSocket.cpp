#include "SslTcpNonblockingSocket.hpp"
#include <format>
#include <iostream>
#include <thread>

using namespace inet;

char SslTcpNonblockingSocket::errBuf[256];

SslTcpNonblockingSocket::SslCtx::SslCtx(const std::string& CertPath, const std::string& PrivKeyPath) {
    const SSL_METHOD* method;

    method = TLS_server_method();

    sslCtx = SSL_CTX_new(method);
    if (!sslCtx) {
        throw std::runtime_error(std::format("Couldn't open ssl context: {}", ERR_error_string(ERR_get_error(), errBuf)));
    }

    if (SSL_CTX_use_certificate_file(sslCtx, CertPath.data(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error(std::format("Couldn't use certificate for socket: {}, certificate path is set to {}", ERR_error_string(ERR_get_error(), errBuf), CertPath));
    }

    if (SSL_CTX_use_PrivateKey_file(sslCtx, PrivKeyPath.data(), SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error(std::format("Couldn't use private key for socket: {}, private key path is set to {}", ERR_error_string(ERR_get_error(), errBuf), PrivKeyPath));
    }
}

SslTcpNonblockingSocket::SslCtx::~SslCtx() {
    SSL_CTX_free(sslCtx);
}

SslTcpNonblockingSocket::SslTcpNonblockingSocket(std::shared_ptr<ISocket> _psock, SSL* _ssl)
	: psock{ _psock }, ssl{_ssl}
{
	;
}

SslTcpNonblockingSocket::~SslTcpNonblockingSocket() {
   
}

int SslTcpNonblockingSocket::init() const {
    return 0;
}

std::pair<ssize_t, std::shared_ptr<ISocket>> SslTcpNonblockingSocket::accept() const {
    auto [err, client] = psock->accept();
    if (client == nullptr) {
        lastErr = { Error::AcceptUnderlyingSocket, -err };
        return { err, nullptr };
    }
    SSL* clientSsl = nullptr;
    if (clientSsl = SSL_new(ctx.sslCtx); clientSsl == 0) {
        lastErr = { Error::AcceptSslNew, ERR_get_error() };
        return {-EINVAL, nullptr};
    }
    if (SSL_set_fd(clientSsl, client->fd()) == 0) {
        lastErr = { Error::AcceptSetFd, ERR_get_error() };
        return { -EBADFD, nullptr };
    }
    for (;;) {
        int status = SSL_accept(clientSsl);
        int err = SSL_get_error(clientSsl, status);
        if (status > 0) {
            break;
        }
        else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)  {
            //std::cout << SSL_state_string_long(clientSsl) << std::endl;
           // std::this_thread::sleep_for(std::chrono::milliseconds(1));
           // continue;
            return { -EAGAIN, std::shared_ptr<ISocket>(new SslTcpNonblockingSocket(client, clientSsl)) };
        }
        else if (status <= 0) {
            //ERR_print_errors_fp(stderr);
            //std::cout << SSL_state_string_long(clientSsl) << std::endl;
            lastErr = { Error::AcceptFail, SSL_get_error(clientSsl, err) };
            return { -EINVAL, nullptr };
        }
    }
    return { 0, std::shared_ptr<ISocket>(new SslTcpNonblockingSocket(client, clientSsl)) };
}

ssize_t SslTcpNonblockingSocket::read(InputSocketBuffer& sockBuf) const {
    size_t nbytes = 0;
    for (;;) {
        ssize_t n = sockBuf.read(&::SSL_read, ssl);
        lastErr.second = SSL_get_error(ssl, n);
        const int& err = lastErr.second;
        if (err != SSL_ERROR_NONE) {
            if (err == SSL_ERROR_ZERO_RETURN) {
                lastErr.first = Error::ReadClientClose;
                return 0;
            }
            else if (err == SSL_ERROR_WANT_READ)
            {
                //Log.debug(std::format("EAGAIN on socket {}", sock.fd()));
                return nbytes ? nbytes : -EAGAIN;
            }
            else {
                // n < 0 - error
                lastErr.first = Error::ReadError;
                return -EBADFD;
            }
        }
        else {
            nbytes += n;
            //break;
        }
    }
    return nbytes;
}

ssize_t SslTcpNonblockingSocket::write(OutputSocketBuffer& sockBuf) const {
    size_t nbytes = 0;
    for (;;) {
        ssize_t n = sockBuf.write(&::SSL_write, ssl);
        lastErr.second = SSL_get_error(ssl, n);
        const int& err = lastErr.second;
        if (err != SSL_ERROR_NONE) {
            if (err == SSL_ERROR_ZERO_RETURN) {
                lastErr.first = Error::WriteClientClose;
                return 0;
            }
            else if (errno == SSL_ERROR_WANT_WRITE) {
                // can't write into socket - sleep and try again
                //Log.debug(std::format("Couldn't write to socket {} - EAGAIN", _fd));
                //std::this_thread::sleep_for(std::chrono::milliseconds(1));
                //continue;
                return nbytes ? nbytes : -EAGAIN;
            }
            else {
                lastErr.first = Error::WriteError;
                return -EBADFD;
            }
        }
        else if (!sockBuf.finished()) {
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

int SslTcpNonblockingSocket::close() {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }
    int res = ::close(psock->fd());
    psock.reset();
    return res;
}

std::string SslTcpNonblockingSocket::strerr() const {
    switch (lastErr.first) {
    case Error::AcceptUnderlyingSocket:
        return psock->strerr();
    case Error::AcceptSslNew:
        return std::format("Couldn't create ssl connection for socket {}: {}", psock->fd(), ERR_error_string(lastErr.second, errBuf));
    case Error::AcceptSetFd:
        return std::format("Couldn't set ssl to fd {}: {}", psock->fd(), ERR_error_string(lastErr.second, errBuf));
    case Error::AcceptFail:
        return std::format("Couldn't accept ssl to fd {}: {}", psock->fd(), ERR_error_string(lastErr.second, errBuf));
    case Error::ReadClientClose:
        return std::format("error reading from socket {}: client has closed connection", psock->fd());
    case Error::ReadError:
        return std::format("error reading from socket {}: {}", psock->fd(), ERR_error_string(lastErr.second, errBuf));
    case Error::WriteClientClose:
        return std::format("error writing to socket {}: client has closed connection", psock->fd());
    case Error::WriteError:
        return std::format("error writing to socket {}: {}", psock->fd(), ERR_error_string(lastErr.second, errBuf));
    default:
        return "No error";
    }
}
