#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <axmol/base/Object.h>

#include <cassert>
#include <utility>

template <typename T>
bim::axmol::ref_ptr<T>::ref_ptr() noexcept
  : m_ptr(nullptr)
{
  static_assert(std::is_base_of_v<ax::Object, T>);
}

template <typename T>
bim::axmol::ref_ptr<T>::ref_ptr(std::nullptr_t) noexcept
  : m_ptr(nullptr)
{
  static_assert(std::is_base_of_v<ax::Object, T>);
}

template <typename T>
bim::axmol::ref_ptr<T>::ref_ptr(T* p) noexcept
  : m_ptr(p)
{
  if (m_ptr)
    m_ptr->retain();
}

template <typename T>
bim::axmol::ref_ptr<T>::ref_ptr(const ref_ptr<T>& p) noexcept
  : m_ptr(p.m_ptr)
{
  if (m_ptr)
    m_ptr->retain();
}

template <typename T>
bim::axmol::ref_ptr<T>::ref_ptr(ref_ptr<T>&& p) noexcept
  : m_ptr(std::exchange(p.m_ptr, nullptr))
{}

template <typename T>
template <typename U>
  requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
bim::axmol::ref_ptr<T>::ref_ptr(U* p) noexcept
  : m_ptr(p)
{
  static_assert(std::is_base_of_v<ax::Object, T>);

  if (m_ptr)
    m_ptr->retain();
}

template <typename T>
template <typename U>
  requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
bim::axmol::ref_ptr<T>::ref_ptr(const ref_ptr<U>& p) noexcept
  : m_ptr(p.get())
{
  static_assert(std::is_base_of_v<ax::Object, T>);

  if (m_ptr)
    m_ptr->retain();
}

template <typename T>
template <typename U>
  requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
bim::axmol::ref_ptr<T>::ref_ptr(ref_ptr<U>&& p) noexcept
  : m_ptr(std::exchange(p.m_ptr, nullptr))
{
  static_assert(std::is_base_of_v<ax::Object, T>);
}

template <typename T>
bim::axmol::ref_ptr<T>::~ref_ptr()
{
  if (m_ptr)
    m_ptr->release();
}

template <typename T>
bim::axmol::ref_ptr<T>&
bim::axmol::ref_ptr<T>::operator=(std::nullptr_t) noexcept
{
  if (m_ptr)
    {
      m_ptr->release();
      m_ptr = nullptr;
    }

  return *this;
}

template <typename T>
template <typename U>
  requires std::is_base_of_v<T, U>
bim::axmol::ref_ptr<T>& bim::axmol::ref_ptr<T>::operator=(U* p) noexcept
{
  if (m_ptr)
    m_ptr->release();

  m_ptr = p;

  if (m_ptr)
    m_ptr->retain();

  return *this;
}

template <typename T>
bim::axmol::ref_ptr<T>&
bim::axmol::ref_ptr<T>::operator=(const ref_ptr& that) noexcept
{
  if (this == &that)
    return *this;

  if (m_ptr)
    m_ptr->release();

  m_ptr = that.m_ptr;

  if (m_ptr)
    m_ptr->retain();

  return *this;
}

template <typename T>
bim::axmol::ref_ptr<T>&
bim::axmol::ref_ptr<T>::operator=(ref_ptr&& that) noexcept
{
  if (this == &that)
    return *this;

  if (m_ptr)
    m_ptr->release();

  m_ptr = std::exchange(that.m_ptr, nullptr);

  return *this;
}

template <typename T>
template <typename U>
  requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
bim::axmol::ref_ptr<T>&
bim::axmol::ref_ptr<T>::operator=(const ref_ptr<U>& that) noexcept
{
  if (m_ptr)
    m_ptr->release();

  m_ptr = that.m_ptr;

  if (m_ptr)
    m_ptr->retain();

  return *this;
}

template <typename T>
template <typename U>
  requires(!std::is_same_v<T, U> && std::is_base_of_v<T, U>)
bim::axmol::ref_ptr<T>&
bim::axmol::ref_ptr<T>::operator=(ref_ptr<U>&& that) noexcept
{
  if (m_ptr)
    m_ptr->release();

  m_ptr = std::exchange(that.m_ptr, nullptr);

  return *this;
}

template <typename T>
T& bim::axmol::ref_ptr<T>::operator*() const
{
  assert(m_ptr != nullptr);
  return *m_ptr;
}

template <typename T>
T* bim::axmol::ref_ptr<T>::operator->() const
{
  assert(m_ptr != nullptr);
  return m_ptr;
}

template <typename T>
T* bim::axmol::ref_ptr<T>::get() const
{
  return m_ptr;
}
