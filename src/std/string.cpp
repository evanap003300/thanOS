#include "std/string.h"
#include "memory/heap.h"

size_t strlen(const char* str) {
	size_t len = 0;
	
	while (str[len] != '\0') {
		len++;
	}

	return len;
}

String::String() {
	this->_length = 0;
	this->buffer = new char[1];
	this->buffer[0] = '\0';
}

String::String(const char* string) {
	this->_length = strlen(string);
	this->buffer = new char[this->_length + 1];
	
	for (size_t i = 0; i < this->_length; i++) {
		this->buffer[i] = string[i];
	}

	this->buffer[this->_length] = '\0';
}

String::String(const String& other) {
	this->_length = other._length;
	this->buffer = new char[this->_length + 1];

	for (size_t i = 0; i < this->_length; i++) {
		this->buffer[i] = other.buffer[i];
	}

	this->buffer[this->_length] = '\0';
}

String::~String() {
	delete[] this->buffer;
}

String& String::operator=(const String& other) {
	if (this == &other) return *this;

	delete[] this->buffer;
	this->_length = other._length;

	this->buffer = new char[this->_length + 1];

	for (size_t i = 0; i < this->_length; i++) {
		this->buffer[i] = other.buffer[i];
	}

	this->buffer[this->_length] = '\0';


	return *this;
}


char& String::operator[](size_t index) {
	return this->buffer[index];
}

size_t String::length() const {
	return this->_length;
}

const char* String::c_str() const {
	return this->buffer;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	while (n > 0 && *s1 != '\0' && (*s1 == *s2)) {
		s1++;
		s2++;
		n--;
	}

	if (n == 0) {
		return 0;
	}

	return *(unsigned char*)s1 - *(unsigned char*)s2;
}

String String::operator+(const String& rhs) {
	String result;
	
	delete[] result.buffer;

	result._length = this->_length + rhs._length;
	result.buffer = new char[result._length + 1];

	size_t i = 0;
	for (; i < this->_length; i++) {
		result.buffer[i] = this->buffer[i];
	}

	for (size_t j = 0; j < rhs._length; j++) {
		result.buffer[i+j] = rhs.buffer[j];
	}

	result.buffer[result._length] = '\0';
	return result;
}

bool String::operator==(const String& other) const {
	if (this->_length != other._length) {
		return false;
	}	
	
	for (size_t i = 0; i < this->_length; i++) {
		if (this->buffer[i] != other.buffer[i]) {
			return false;
		}
	}

	return true;
}

bool String::operator==(const char* other) const {
	if (other == nullptr) {
		return (this->_length == 0);
	}

	size_t i = 0;
	
	while (i < this->_length && other[i] != '\0') {
		if (this->buffer[i] != other[i]) {
			return false;
		}
		i++;
	}

	return (i == this->_length && other[i] == '\0');
}
