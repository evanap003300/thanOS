#pragma once
#include <stdint.h>
#include <stddef.h>

class String {
private:
	char* buffer;
	size_t _length;

public:
	String();
	String(const char* string);
	String(const String& other);
	~String();

	String& operator=(const String& other);

	char& operator[](size_t index);

	size_t length() const;
	const char* c_str() const; 

	friend String operator+(const String& lhs, const String& rhs);
};

