#pragma once

#include <cstdint>
#include <cstdlib>

template <typename T>
struct view
{
public:
    view(T* data, const size_t count) noexcept : fdata(data), fsize(count) {}
    view(const view<T>& other) noexcept : fdata(other.fdata), fsize(other.fsize) {}
    view& operator=(const view<T>& other)
    {
        fdata = other.fdata;
        fsize = other.fsize;
        return *this;
    }

    inline FORCEINLINE const view<T> slice(size_t start, size_t count) const noexcept
    {
        return view<T>(data() + start, count);
    }

    inline FORCEINLINE view<T> slice(size_t start, size_t count) noexcept
    {
        return view<T>(data() + start, count);
    }

    inline FORCEINLINE size_t size() const noexcept { return fsize; }
    inline FORCEINLINE const T* data(const size_t offset) const noexcept { return fdata + offset; }
    inline FORCEINLINE const T* data() const noexcept { return data(0); }
    inline FORCEINLINE T* data(const size_t offset) noexcept { return fdata + offset; }
    inline FORCEINLINE T* data() noexcept { return data(0); }

    inline FORCEINLINE T operator[](size_t index) const noexcept { return fdata[index]; }

    const char* type() const noexcept { return "view"; }

private:
    T* fdata;
    size_t fsize;
};
