// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <cstdint>
#include <string_view>

namespace bim::app
{
  class analytics_service;

  void coins_transaction(analytics_service& service, std::string_view cause,
                         std::int32_t amount);
}
