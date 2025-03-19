// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/server/tests/new_test_config.hpp>

bim::server::config bim::server::tests::new_test_config()
{
  static unsigned short port = 10000;

  bim::server::config config;
  config.port = port;
  ++port;

  return config;
}
