#pragma once

#include "common.hpp"

#include <cstring>

template <typename T>
struct vuctor
{
    vuctor() noexcept = default;
    vuctor(const T* start, const T* end) noexcept { assign(start, end); }

    ~vuctor() noexcept { free(_data); }

    vuctor(vuctor<T>&& other) noexcept { *this = std::move(other); }
    vuctor& operator=(vuctor<T>&& other) noexcept
    {
        free(_data);
        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;
        other._data = nullptr;
        other._size = other._capacity = 0;
        return *this;
    }

    vuctor(const vuctor<T>& other) = delete;
    vuctor& operator=(const vuctor<T>& other) = delete;

    void reserve(size_t entries) noexcept
    {
        if (_capacity < entries)
        {
            DEBUGLOG("going to realloc %zu bytes\n", entries * sizeof(T));
            _data = (T*)realloc(_data, entries * sizeof(T));
            _capacity = entries;
        }
    }

    void resize(size_t entries) noexcept
    {
        reserve(entries);
        _size = entries;
    }

    void assign(const T* start, const T* end) noexcept
    {
        resize(end - start);
        memcpy(_data, start, (end - start) * sizeof(T));
    }

    size_t size() const noexcept { return _size; }
    size_t capacity() const noexcept { return _capacity; }
    const T* data() const noexcept { return _data; }
    T* data() noexcept { return _data; }

    const T& operator[](size_t index) const noexcept { return _data[index]; }
    T& operator[](size_t index) noexcept { return _data[index]; }

private:
    T* _data = nullptr;
    size_t _size = 0;
    size_t _capacity = 0;
};
