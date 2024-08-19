// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <algorithm>

template <typename T>
void bim::game::input_archive::operator()(T& value)
{
  char* data = reinterpret_cast<char*>(std::addressof(value));
  std::copy(m_cursor, m_cursor + sizeof(T), data);
  m_cursor += sizeof(T);
}

template <typename T>
void bim::game::input_archive::operator()(entt::entity& entity, T& value)
{
  operator()(entity);
  operator()(value);
}
