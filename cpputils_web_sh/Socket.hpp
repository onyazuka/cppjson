#pragma once
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <vector>
#include <span>
#include <memory>
#include <unordered_map>
#include <string>

namespace inet {

	class InputSocketBuffer {
	public:
		InputSocketBuffer(size_t minCapacity = 100, size_t capacity = 10 * 1024, size_t maxCapacity = 1 * 1024 * 1024);
		void clear();
		void clear(size_t n);
		inline size_t size() const { return _size; }
		inline size_t capacity() const { return _capacity; }
		template<typename ReadFT, typename ArgT>
		ssize_t read(ReadFT readF, ArgT fd);
		std::span<uint8_t> get();
	private:
		std::span<uint8_t> getTail();
		void realloc(size_t newCap);
		void realloc();
		std::shared_ptr<uint8_t[]> _data;
		size_t _size = 0;
		size_t _capacity = 0;
		// should be no less than minSizeAvail each time before we read into buffer
		const size_t MinSizeAvail;
		const size_t MaxCapacity;
	};

	template<typename ReadFT, typename ArgT>
	ssize_t InputSocketBuffer::read(ReadFT readF, ArgT fd) {
		if ((_capacity - _size) < MinSizeAvail) {
			realloc();
		}
		auto data = getTail();
		ssize_t nbytes = 0;
		if (nbytes = readF(fd, data.data(), data.size()); nbytes > 0) {
			_size += nbytes;
		}
		return nbytes;
	}

	class OutputSocketBuffer {
	public:
		OutputSocketBuffer();
		OutputSocketBuffer(std::string&& sdata);
		template<typename WriteFT, typename ArgT>
		// returns true if all data written, else false
		ssize_t write(WriteFT writeF, ArgT fd);
		inline bool finished() const { return _offset == _data.size(); }
		inline bool empty() const { return _data.empty(); }
		inline size_t offset() const { return _offset; }
		inline size_t size() const { return _data.size(); }
		inline void clear() { _data.clear(); _offset = 0; }
	private:
		std::string _data;
		size_t _offset = 0;
	};

	template<typename WriteFT, typename ArgT>
	ssize_t OutputSocketBuffer::write(WriteFT writeF, ArgT fd) {
		ssize_t nbytes = writeF(fd, _data.data() + _offset, _data.size() - _offset);
		if (nbytes > 0) {
			_offset += nbytes;
		}
		return nbytes;
	}
	
	class ISocket {
	public:
		virtual ~ISocket();
		virtual int init() const = 0;
		virtual std::pair<ssize_t, std::shared_ptr<ISocket>> accept() const = 0;
		std::pair<bool, std::vector<std::pair<ssize_t, std::shared_ptr<ISocket>>>> acceptAll() const;
		virtual ssize_t read(InputSocketBuffer& buf) const = 0;
		virtual ssize_t write(OutputSocketBuffer& buf) const = 0;
		//virtual int shutdown(int flags) = 0;
		virtual int close() = 0;
		virtual int fd() const = 0;
		virtual std::string strerr() const = 0;
	};

}

template <>
struct std::hash<std::shared_ptr<inet::ISocket>>
{
	std::size_t operator()(std::shared_ptr<inet::ISocket> ptr) const
	{
		return std::hash<int>()(ptr->fd());
	}
};
