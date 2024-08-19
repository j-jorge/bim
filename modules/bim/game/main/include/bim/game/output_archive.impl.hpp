// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

template <typename T>
void bim::game::output_archive::operator()(const T& value) const
{
  const char* data = reinterpret_cast<const char*>(std::addressof(value));
  m_storage.insert(m_storage.end(), data, data + sizeof(T));
}

template <typename T>
void bim::game::output_archive::operator()(entt::entity entity,
                                           const T& value) const
{
  m_storage.reserve(m_storage.size() + sizeof(entity) + sizeof(T));

  operator()(entity);
  operator()(value);
}
