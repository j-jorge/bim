// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <string_view>

namespace bim::app
{
  class analytics_service;

  void error(analytics_service& service, std::string_view cause);
}
