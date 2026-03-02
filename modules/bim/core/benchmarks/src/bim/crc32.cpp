// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/crc32.hpp>

#include <benchmark/benchmark.h>

static void crc32(benchmark::State& state)
{
  std::vector<char> v(state.range(0));
  for (std::size_t i = 0; i != v.size(); ++i)
    v[i] = i;

  for (auto _ : state)
    {
      std::uint32_t r = bim::crc32(v);
      benchmark::DoNotOptimize(r);
    }
}

BENCHMARK(crc32)
    ->Arg(1 << 15)
    ->Arg(1 << 16)
    ->Arg(1 << 17)
    ->Arg(1 << 18)
    ->Arg(1 << 19)
    ->Arg(1 << 20);
