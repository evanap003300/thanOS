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
	int strncmp(const char* s1, const char* s2, size_t n);

	String operator+(const String& rhs);
	bool operator==(const String& other) const;
	bool operator==(const char* other) const;
};

int strncmp(const char* s1, const char* s2, size_t n);
size_t strlen(const char* str);
