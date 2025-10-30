// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/analytics/button_clicked.hpp>

#include <bim/app/analytics_service.hpp>

void bim::app::button_clicked(analytics_service& service, std::string_view id,
                              std::string_view where)
{
  service.event("button", { { "id", id }, { "where", where } });
}
