#include "newVector.hpp"

template <typename T>
newVector<T>::newVector() : size_(0), capacity_(0), data_(nullptr) {}

template <typename T>
newVector<T>::~newVector() {
    clear();
    operator delete(data_);
}

template <typename T>
void newVector<T>::push_back(const T& value) {
    if (size_ == capacity_) {
        reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }

    new (data_ + size_) T(value);
    size_++;
}

template <typename T>
void newVector<T>::pop_back() {
    if (size_ > 0) {
        size_--;
        data_[size_].~T();
    }
}

template <typename T>
void newVector<T>::clear() {
    for (size_t i = 0; i < size_; i++) {
        data_[i].~T();
    }

    size_ = 0;
}

template <typename T>
void newVector<T>::reserve(size_t new_capacity) {
    if (new_capacity > capacity_) {
        T* new_data = static_cast<T*>(operator new(new_capacity * sizeof(T)));

        for (size_t i = 0; i < size_; i++) {
            new (new_data + i) T(std::move(data_[i]));
            data_[i].~T();
        }

        operator delete(data_);
        data_ = new_data;
        capacity_ = new_capacity;
    }
}

template <typename T>
size_t newVector<T>::size() const {
    return size_;
}

template <typename T>
size_t newVector<T>::capacity() const {
    return capacity_;
}

template <typename T>
T& newVector<T>::operator[](size_t index) {
    return data_[index];
}

template <typename T>
const T& newVector<T>::operator[](size_t index) const {
    return data_[index];
}
template <typename T>
T* newVector<T>::begin() {
    return data_;
}

template <typename T>
const T* newVector<T>::begin() const {
    return data_;
}
template <typename T>
T* newVector<T>::end() {
    return data_ + size_;
}

template <typename T>
const T* newVector<T>::end() const {
    return data_ + size_;
}