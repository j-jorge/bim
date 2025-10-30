// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/analytics/coins_transaction.hpp>

#include <bim/app/analytics_service.hpp>

#include <charconv>

void bim::app::coins_transaction(analytics_service& service,
                                 std::string_view cause, std::int32_t amount)
{
  char a[10];
  const std::to_chars_result r =
      std::to_chars(std::begin(a), std::end(a), std::abs(amount));

  if (r.ec != std::errc{})
    return;

  const std::string_view amount_str(a, r.ptr);
  service.event(amount < 0 ? "debit" : "credit", { { "currency", "coins" },
                                                   { "cause", cause },
                                                   { "amount", amount_str } });
}
