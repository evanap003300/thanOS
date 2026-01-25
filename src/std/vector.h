#pragma once
#include <stdint.h>
#include <stddef.h>
#include "memory/heap.h"

template <typename T>
class Vector {
private:
	T* _data;
	size_t _capacity;
	size_t _size;

	void resize() {
		size_t new_capacity = (_capacity == 0) ? 2 : _capacity * 2;
		T* new_data = new T[new_capacity];

		for (size_t i = 0 ; i < _size; i++) {
			new_data[i] = _data[i];
		}

		if (_data != NULL) {
			delete[] _data;
		}

		_data = new_data;
		_capacity = new_capacity;
	}

public:
	Vector() {
		_data = NULL;
		_size = 0;
		_capacity = 0;
	}

	~Vector() {
		if (_data != NULL) {
			delete[] _data;
		}
	}

	void push_back(T element) {
		if (_size >= _capacity) {
			resize();
		}
		_data[_size] = element;
		_size++;
	}

	T& operator[](size_t index) {
		return _data[index];
	}

	size_t size() const {
		return _size;
	}

	void remove(size_t index) {
		if (index >= _size) {
			return;
		}

		for (size_t i = index; i < _size - 1; i++) { 
			_data[i] = _data[i+1];
		}

		_size--;
	}

	void clear() {
		_size = 0;
	}
};
