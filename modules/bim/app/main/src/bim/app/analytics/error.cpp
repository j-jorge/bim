// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/analytics/error.hpp>

#include <bim/app/analytics_service.hpp>

void bim::app::error(analytics_service& service, std::string_view cause)
{
  service.event("error", { { "cause", cause } });
}
