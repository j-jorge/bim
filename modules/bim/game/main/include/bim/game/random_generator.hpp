// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>
#include <limits>

namespace bim::game
{
  /**
   * Random integer generator using xoshiro256++.
   */
  class random_generator
  {
  public:
    using result_type = std::uint64_t;

  public:
    explicit random_generator(std::uint64_t seed);

    result_type operator()();

    static constexpr result_type min()
    {
      return std::numeric_limits<result_type>::lowest();
    }

    static constexpr result_type max()
    {
      return std::numeric_limits<result_type>::max();
    }

  private:
    std::uint64_t m_state[4];
  };
}
