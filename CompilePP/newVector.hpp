#pragma once
#include <utility>
template <typename T>
class newVector {
public:
    newVector();

    ~newVector();

    void push_back(const T& value);

    void pop_back();

    void clear();

    void reserve(size_t new_capacity);

    size_t size() const;

    size_t capacity() const;

    T& operator[](size_t index);

    const T& operator[](size_t index) const;

    T* begin();

    const T* begin() const;

    T* end();

    const T* end() const;

private:
    size_t size_;
    size_t capacity_;
    T* data_;
};