/*
  Copyright (C) 2023 Julien Jorge

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <bm/game/random_generator.hpp>

bm::game::random_generator::random_generator(std::uint64_t seed)
  : m_state{ seed, 0, 0, 0 }
{}

bm::game::random_generator::result_type
bm::game::random_generator::operator()()
{
  // This is the implementation from
  // https://prng.di.unimi.it/xoshiro256plusplus.c.

  constexpr auto rotl = [](std::uint64_t x, int k) -> std::uint64_t
  {
    return (x << k) | (x >> (64 - k));
  };

  const std::uint64_t result = rotl(m_state[0] + m_state[3], 23) + m_state[0];
  const std::uint64_t t = m_state[1] << 17;

  m_state[2] ^= m_state[0];
  m_state[3] ^= m_state[1];
  m_state[1] ^= m_state[2];
  m_state[0] ^= m_state[3];

  m_state[2] ^= t;

  m_state[3] = rotl(m_state[3], 45);

  return result;
}
