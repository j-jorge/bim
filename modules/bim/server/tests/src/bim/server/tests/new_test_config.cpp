// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/new_test_config.hpp>

bim::server::config bim::server::tests::new_test_config()
{
  static unsigned short port = 10000;

  bim::server::config config;
  config.port = port;
  ++port;

  config.statistics_dump_delay = std::chrono::seconds(0);

  return config;
}
