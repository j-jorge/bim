// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cassert>

template <typename T>
bim::axmol::input::backend_event<T>::backend_event(T data)
  : m_data(data)
  , m_is_available(true)
{}

template <typename T>
void bim::axmol::input::backend_event<T>::consume()
{
  assert(is_available());
  m_is_available = false;
}

template <typename T>
bool bim::axmol::input::backend_event<T>::is_available() const
{
  return m_is_available;
}

template <typename T>
T bim::axmol::input::backend_event<T>::get() const
{
  return m_data;
}
