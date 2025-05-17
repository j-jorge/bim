// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/bit_map.impl.hpp>

#include <gtest/gtest.h>

TEST(bim_bit_map, eight_bits)
{
  enum test : std::uint8_t
  {
  };
  bim::bit_map<test, char> m;

  m[test(0)] = 'a';
  m[test(1 << 0)] = 'b';
  m[test(1 << 1)] = 'c';
  m[test(1 << 2)] = 'd';
  m[test(1 << 3)] = 'e';
  m[test(1 << 4)] = 'f';
  m[test(1 << 5)] = 'g';
  m[test(1 << 6)] = 'h';
  m[test(1 << 7)] = 'i';

  EXPECT_EQ('a', m[test(0)]);
  EXPECT_EQ('b', m[test(1 << 0)]);
  EXPECT_EQ('c', m[test(1 << 1)]);
  EXPECT_EQ('d', m[test(1 << 2)]);
  EXPECT_EQ('e', m[test(1 << 3)]);
  EXPECT_EQ('f', m[test(1 << 4)]);
  EXPECT_EQ('g', m[test(1 << 5)]);
  EXPECT_EQ('h', m[test(1 << 6)]);
  EXPECT_EQ('i', m[test(1 << 7)]);
}
