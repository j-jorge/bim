// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/tracy.hpp>

#include <new>

#include <stdlib.h>

void* operator new(std::size_t count)
{
  if (count == 0)
    count = 1;

  void* const r = malloc(count);

  if (!r)
    throw std::bad_alloc();

  TracyAlloc(r, count);
  return r;
}

void* operator new[](std::size_t count)
{
  if (count == 0)
    count = 1;

  void* const r = malloc(count);

  if (!r)
    throw std::bad_alloc();

  TracyAlloc(r, count);
  return r;
}

void* operator new(std::size_t count, std::align_val_t al)
{
  if (count == 0)
    count = 1;

  void* r;

  if (posix_memalign(&r, (size_t)al, count) != 0)
    throw std::bad_alloc();

  TracyAlloc(r, count);
  return r;
}

void* operator new[](std::size_t count, std::align_val_t al)
{
  if (count == 0)
    count = 1;

  void* r;

  if (posix_memalign(&r, (size_t)al, count) != 0)
    throw std::bad_alloc();

  TracyAlloc(r, count);
  return r;
}

void* operator new(std::size_t count, const std::nothrow_t& tag) noexcept
{
  if (count == 0)
    count = 1;

  void* const r = malloc(count);

  TracyAlloc(r, count);
  return r;
}

void* operator new[](std::size_t count, const std::nothrow_t& tag) noexcept
{
  if (count == 0)
    count = 1;

  void* const r = malloc(count);

  TracyAlloc(r, count);
  return r;
}

void* operator new(std::size_t count, std::align_val_t al,
                   const std::nothrow_t& tag) noexcept
{
  if (count == 0)
    count = 1;

  void* r;

  if (posix_memalign(&r, (size_t)al, count) != 0)
    return nullptr;

  TracyAlloc(r, count);
  return r;
}

void* operator new[](std::size_t count, std::align_val_t al,
                     const std::nothrow_t& tag) noexcept
{
  if (count == 0)
    count = 1;

  void* r;

  if (posix_memalign(&r, (size_t)al, count) != 0)
    return nullptr;

  TracyAlloc(r, count);
  return r;
}

void operator delete(void* ptr) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete[](void* ptr) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete(void* ptr, std::align_val_t al) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete[](void* ptr, std::align_val_t al) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete(void* ptr, std::size_t sz) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete[](void* ptr, std::size_t sz) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete(void* ptr, std::size_t sz, std::align_val_t al) noexcept
{
  TracyFree(ptr);
  free(ptr);
}

void operator delete[](void* ptr, std::size_t sz, std::align_val_t al) noexcept
{
  TracyFree(ptr);
  free(ptr);
}
