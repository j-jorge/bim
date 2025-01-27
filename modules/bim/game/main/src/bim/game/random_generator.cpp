// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/random_generator.hpp>

bim::game::random_generator::random_generator(std::uint64_t seed)
{
  // Initialization with splitmix64.
  const std::uint64_t phi = 0x9e3779b97f4a7c15;

  for (int i = 0; i != 4; ++i)
    {
      seed += phi;
      std::uint64_t z = seed;
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
      z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
      z = z ^ (z >> 31);
      m_state[i] = z;
    }
}

bim::game::random_generator::result_type
bim::game::random_generator::operator()()
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
