// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/bit_map.hpp>

#include <bit>
#include <cassert>

template <typename E, typename T, int M>
constexpr const T& bim::bit_map<E, T, M>::operator[](E v) const
{
  assert(std::popcount(std::underlying_type_t<E>(v)) <= 1);
  assert(std::countr_zero(std::underlying_type_t<E>(v)) < M);
  return m_data[std::countr_zero(std::underlying_type_t<E>(v))];
}

template <typename E, typename T, int M>
constexpr T& bim::bit_map<E, T, M>::operator[](E v)
{
  assert(std::popcount(std::underlying_type_t<E>(v)) <= 1);
  assert(std::countr_zero(std::underlying_type_t<E>(v)) < M);
  return m_data[std::countr_zero(std::underlying_type_t<E>(v))];
}
