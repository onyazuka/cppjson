#include "Socket.hpp"
#include <cassert>
#include <string.h>
#include <fcntl.h>
#include <format>
#include <iostream>

using namespace inet;

InputSocketBuffer::InputSocketBuffer(size_t minSizeAvail, size_t capacity, size_t maxCapacity)
	: _size{ 0 }, _capacity{ capacity }, MinSizeAvail{ minSizeAvail }, MaxCapacity{ maxCapacity }
{
	assert(_capacity <= MaxCapacity);
	_data = std::shared_ptr<uint8_t[]>(new uint8_t[_capacity]);
}

std::span<uint8_t> InputSocketBuffer::get() {
	return std::span<uint8_t>(_data.get(), _size);
}

std::span<uint8_t> InputSocketBuffer::getTail() {
	return std::span<uint8_t>(_data.get() + _size, _capacity - _size);
}

void InputSocketBuffer::realloc(size_t newCap) {
	assert(newCap <= MaxCapacity && newCap > _capacity);
	std::shared_ptr<uint8_t[]> newData(new uint8_t[newCap]);
	memcpy(newData.get(), _data.get(), _size);
	_capacity = newCap;
	_data = newData;
}

void InputSocketBuffer::realloc() {
	realloc(std::min(MaxCapacity, std::max((size_t)0, (_capacity + MinSizeAvail) * 2)));
}

void InputSocketBuffer::clear() {
	_size = 0;
}

// clears n bytes and moves rest of bytes to the beginning of the buffer
void InputSocketBuffer::clear(size_t n) {
	assert(_size >= n);
	memmove(_data.get(), _data.get() + n, _size - n);
	_size = _size - n;
}

OutputSocketBuffer::OutputSocketBuffer() {
	;
}

OutputSocketBuffer::OutputSocketBuffer(std::string&& sdata)
	: _data{ std::move(sdata) }
{
	;
}

ISocket::~ISocket() {
	;
}

std::pair<bool, std::vector<std::pair<ssize_t, std::shared_ptr<ISocket>>>> ISocket::acceptAll() const {
	std::vector<std::pair<ssize_t, std::shared_ptr<ISocket>>> fds;
	std::shared_ptr<ISocket> fd = 0;
	ssize_t errNum;
	bool err = false;
	do {
		std::tie(errNum, fd) = accept();
		if (fd == nullptr) {
			if (errNum != -EAGAIN) err = true;
			break;
		}
		fds.push_back({ errNum,fd});
	} while (fd != nullptr);
	return { err, fds };
}
