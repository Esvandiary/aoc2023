#pragma once

#include "common.h"
#include "view.h"

#if defined(_WIN32)

#define MMAPFILE_AVAILABLE

#if !defined(NOMINMAX)
    #define NOMINMAX
#endif
#include <windows.h>

typedef struct mmap_file
{
    HANDLE file;
    HANDLE mapping;

    chartype* data;
    size_t size;
} mmap_file;

static mmap_file mmap_file_open_ro(const char* filename)
{
    auto file = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return mmap_file { .file = NULL, .mapping = NULL, .data = NULL, .size = 0 };
    LARGE_INTEGER size;
    if (!::GetFileSizeEx(file, &size))
        return mmap_file { .file = NULL, .mapping = NULL, .data = NULL, .size = 0 };
    auto fsize = size_t(size.QuadPart);
    DEBUGLOG("going to mmap %zu bytes\n", fsize);
    auto mapping = ::CreateFileMappingA(file, nullptr, PAGE_WRITECOPY, 0, 0, nullptr);
    if (!mapping)
        return mmap_file { .file = NULL, .mapping = NULL, .data = NULL, .size = 0 };
    auto fdata = (chartype*)::MapViewOfFile(mapping, FILE_MAP_COPY, 0, 0, 0);
    if (!fdata)
        return mmap_file { .file = NULL, .mapping = NULL, .data = NULL, .size = 0 };
    return mmap_file { .file = file, .mapping = mapping, .data = fdata, .size = fsize };
}

static void mmap_file_destroy(mmap_file* file)
{
    if (file->data)
        ::UnmapViewOfFile(file->data);
    if (file->mapping)
        ::CloseHandle(file->mapping);
    if (file->file)
        ::CloseHandle(file->file);
    memset(file, 0, sizeof(mmap_file));
}

#elif defined(__linux__)

#define MMAPFILE_AVAILABLE

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct mmap_file
{
    int fd;
    chartype* data;
    size_t size;
} mmap_file;

static mmap_file mmap_file_open_ro(const char* filename)
{
    auto fd = open(filename, O_RDONLY);
    if (fd < 0)
        return mmap_file { .fd = 0, .data = NULL, .size = 0 };
    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if (err < 0)
        return mmap_file { .fd = 0, .data = NULL, .size = 0 };
    auto fsize = statbuf.st_size;
    DEBUGLOG("going to mmap %zu bytes\n", fsize);
    auto fdata = (chartype*)mmap(nullptr, fsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (fdata == MAP_FAILED)
        return mmap_file { .fd = 0, .data = NULL, .size = 0 };
    return mmap_file { .fd = fd, .data = fdata, .size = fsize };
}

static void mmap_file_destroy(mmap_file* file)
{
    if (file->data)
        munmap((void*)file->data, file->size);
    if (file->fd >= 0)
        close(file->fd);
    memset(file, 0, sizeof(mmap_file));
}

#endif
