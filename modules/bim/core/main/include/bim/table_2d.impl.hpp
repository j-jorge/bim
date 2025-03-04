// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/table_2d.hpp>

#include <bim/assume.hpp>

#include <algorithm>

template <typename T>
bim::table_2d<T>::table_2d()
  : m_width(0)
  , m_height(0)
{}

template <typename T>
bim::table_2d<T>::table_2d(std::size_t w, std::size_t h)
  : m_width(w)
  , m_height(h)
  , m_data(new T[w * h])
{}

template <typename T>
bim::table_2d<T>::table_2d(std::size_t w, std::size_t h, T&& init)
  : table_2d(w, h)
{
  std::fill_n(m_data.get(), w * h, init);
}

template <typename T>
bim::table_2d<T>::table_2d(const table_2d<T>& that)
  : m_width(that.m_width)
  , m_height(that.m_height)
{
  if (!that.m_data)
    return;

  m_data.reset(new T[m_width * m_height]);

  std::copy_n(that.m_data.get(), m_width * m_height, m_data.get());
}

template <typename T>
bim::table_2d<T>::table_2d(table_2d<T>&& that) noexcept = default;

template <typename T>
bim::table_2d<T>& bim::table_2d<T>::operator=(const table_2d<T>& that)
{
  if (this == &that)
    return *this;

  if (!that.m_data)
    {
      m_data.reset();
      return *this;
    }

  if (m_width * m_height < that.m_width * that.m_height)
    m_data.reset(new T[that.m_width * that.m_height]);

  m_width = that.m_width;
  m_height = that.m_height;

  std::copy_n(that.m_data.get(), m_width * m_height, m_data.get());

  return *this;
}

template <typename T>
bim::table_2d<T>&
bim::table_2d<T>::operator=(table_2d<T>&& that) noexcept = default;

template <typename T>
bim::table_2d<T>::table_2d::~table_2d() = default;

template <typename T>
T& bim::table_2d<T>::operator()(std::size_t x, std::size_t y)
{
  bim_assume(m_data);
  bim_assume(x < m_width);
  bim_assume(y < m_height);
  bim_assume(y * m_width + x < m_width * m_height);

  return m_data[y * m_width + x];
}

template <typename T>
const T& bim::table_2d<T>::operator()(std::size_t x, std::size_t y) const
{
  bim_assume(m_data);
  bim_assume(x < m_width);
  bim_assume(y < m_height);
  bim_assume(y * m_width + x < m_width * m_height);

  return m_data[y * m_width + x];
}

template <typename T>
std::size_t bim::table_2d<T>::width() const
{
  return m_width;
}

template <typename T>
std::size_t bim::table_2d<T>::height() const
{
  return m_height;
}

template <typename T>
void bim::table_2d<T>::fill(const T& v)
{
  std::fill_n(m_data.get(), m_width * m_height, v);
}

template <typename T>
T* bim::table_2d<T>::begin()
{
  return m_data.get();
}

template <typename T>
T* bim::table_2d<T>::begin() const
{
  return m_data.get();
}

template <typename T>
T* bim::table_2d<T>::end()
{
  return m_data.get() + m_width * m_height;
}

template <typename T>
T* bim::table_2d<T>::end() const
{
  return m_data.get() + m_width * m_height;
}
