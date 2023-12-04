#pragma once

#include <algorithm>
#include <stdexcept>
#include "common.hpp"
#include "view.hpp"

#if defined(_WIN32)

#define MMAPFILE_AVAILABLE

#define NOMINMAX
#include <windows.h>

struct mmap_file
{
    static mmap_file open_ro(const char* filename)
    {
        auto file = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            throw std::runtime_error("failed to open file");
        LARGE_INTEGER size;
        if (!::GetFileSizeEx(file, &size))
            throw std::runtime_error("failed to get file size");
        auto fsize = size_t(size.QuadPart);
        DEBUGLOG("going to mmap %zu bytes\n", fsize);
        auto mapping = ::CreateFileMappingA(file, nullptr, PAGE_WRITECOPY, 0, 0, nullptr);
        if (!mapping)
            throw std::runtime_error("failed to create file mapping");
        auto fdata = (chartype*)::MapViewOfFile(mapping, FILE_MAP_COPY, 0, 0, 0);
        if (!fdata)
            throw std::runtime_error("failed to map file");
        return mmap_file(file, mapping, fdata, fsize);
    }

    static mmap_file open_rw(const char* filename, size_t start, size_t count)
    {
        auto file = ::CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!file)
            throw std::runtime_error("failed to open file");
        LARGE_INTEGER size;
        if (!::GetFileSizeEx(file, &size))
            throw std::runtime_error("failed to get file size");
        size_t fsize = (std::min)(count, size_t(size.QuadPart));
        DEBUGLOG("going to mmap %zu bytes\n", fsize);
        auto mapping = ::CreateFileMappingA(file, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (!mapping)
            throw std::runtime_error("failed to create file mapping");
        auto fdata = (chartype*)::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, (DWORD)(start >> 32), (DWORD)(start & 0xFFFFFFFF), fsize);
        if (!fdata)
            throw std::runtime_error("failed to map file");
        return mmap_file(file, mapping, fdata, fsize);
    }

    static mmap_file create(const char* filename, size_t sz)
    {
        create_file(filename, sz);
        auto file = ::CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!file)
            throw std::runtime_error("failed to open file");
        DEBUGLOG("going to mmap %zu bytes\n", sz);
        auto mapping = ::CreateFileMappingA(file, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (!mapping)
            throw std::runtime_error("failed to create file mapping");
        auto fdata = (chartype*)::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!fdata)
            throw std::runtime_error("failed to map file");
        return mmap_file(file, mapping, fdata, sz);
    }

    static void create_file(const char* path, size_t sz)
    {
        auto file = ::CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!file)
            throw std::runtime_error("failed to create file");
        LARGE_INTEGER lsz;
        lsz.QuadPart = sz;
        if (::SetFilePointer(file, lsz.LowPart, &lsz.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            throw std::runtime_error("failed to set file pointer");
        if (!::SetEndOfFile(file))
            throw std::runtime_error("failed to resize file");
        if (::SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            throw std::runtime_error("failed to seek back to start of file");
        ::CloseHandle(file);
    }

    ~mmap_file()
    {
        if (fdata)
            ::UnmapViewOfFile(fdata);
        if (mapping)
            ::CloseHandle(mapping);
        if (file)
            ::CloseHandle(file);
    }

    inline FORCEINLINE const view<chartype> slice(size_t start, size_t count) const noexcept
    {
        return view<chartype>(fdata + start, count);
    }

    inline FORCEINLINE view<chartype> slice(size_t start, size_t count) noexcept
    {
        return view<chartype>(data() + start, count);
    }

    inline FORCEINLINE const chartype* data(size_t offset) const noexcept { return fdata + offset; }
    inline FORCEINLINE const chartype* data() const noexcept { return data(0); }
    inline FORCEINLINE chartype* data(size_t offset) noexcept { return fdata + offset; }
    inline FORCEINLINE chartype* data() noexcept { return data(0); }
    inline FORCEINLINE size_t size() const noexcept { return fsize; }

    const char* type() const noexcept { return "mmapfile"; }

private:
    mmap_file(HANDLE file, HANDLE mapping, chartype* fdata, size_t fsize)
        : file(file), mapping(mapping), fdata(fdata), fsize(fsize)
    {
    }

    HANDLE file = 0;
    HANDLE mapping = 0;
    chartype* fdata = 0;
    size_t fsize = 0;
};

#elif defined(__linux__)

#define MMAPFILE_AVAILABLE

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct mmap_file
{
    static mmap_file open_ro(const char* filename)
    {
        auto fd = open(filename, O_RDONLY);
        if (fd < 0)
            throw std::runtime_error("failed to open file");
        struct stat statbuf;
        int err = fstat(fd, &statbuf);
        if (err < 0)
            throw std::runtime_error("failed to get file size");
        auto fsize = statbuf.st_size;
        DEBUGLOG("going to mmap %zu bytes\n", fsize);
        auto fdata = (chartype*)mmap(nullptr, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (fdata == MAP_FAILED)
            throw std::runtime_error("failed to map file");
        return mmap_file(fd, fdata, fsize);
    }

    static mmap_file open_rw(const char* filename, size_t start, size_t count)
    {
        auto fd = open(filename, O_RDWR);
        if (fd < 0)
            throw std::runtime_error("failed to open file");
        struct stat statbuf;
        int err = fstat(fd, &statbuf);
        if (err < 0)
            throw std::runtime_error("failed to get file size");
        auto fsize = (std::min)(count, statbuf.st_size - start);
        DEBUGLOG("going to mmap %zu bytes\n", fsize);
        auto fdata = (chartype*)mmap(nullptr, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
        if (fdata == MAP_FAILED)
            throw std::runtime_error("failed to map file");
        return mmap_file(fd, fdata, fsize);
    }

    static mmap_file create(const char* filename, size_t sz)
    {
        create_file(filename, sz);
        auto fd = open(filename, O_RDWR);
        auto fsize = sz;
        DEBUGLOG("going to mmap %zu bytes\n", fsize);
        auto fdata = (chartype*)mmap(nullptr, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (fdata == MAP_FAILED)
            throw std::runtime_error("failed to map file");
        return mmap_file(fd, fdata, fsize);
    }

    static void create_file(const char* filename, size_t sz)
    {
        int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0)
            throw std::runtime_error("failed to create file");
        if (ftruncate(fd, sz) != 0)
            throw std::runtime_error("failed to resize file");
        close(fd);
    }

    ~mmap_file()
    {
        if (fdata)
            munmap((void*)fdata, fsize);
        if (fd >= 0)
            close(fd);
    }

    inline FORCEINLINE view<chartype> slice(size_t start, size_t count) const noexcept
    {
        return view<chartype>(fdata + start, count);
    }

    inline FORCEINLINE const chartype* data(size_t offset) const noexcept { return fdata + offset; }
    inline FORCEINLINE const chartype* data() const noexcept { return data(0); }
    inline FORCEINLINE chartype* data(size_t offset) noexcept { return fdata + offset; }
    inline FORCEINLINE chartype* data() noexcept { return data(0); }
    inline FORCEINLINE size_t size() const noexcept { return fsize; }

    const char* type() const noexcept { return "mmapfile"; }

private:
    mmap_file(int fd, chartype* fdata, size_t fsize)
        : fd(fd), fdata(fdata), fsize(fsize)
    {
    }

    int fd = -1;
    chartype* fdata = 0;
    size_t fsize = 0;
};

#endif
