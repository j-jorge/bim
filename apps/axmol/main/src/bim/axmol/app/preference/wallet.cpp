// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/preference/wallet.hpp>

#include <iscool/preferences/local_preferences.hpp>

#include <cassert>

std::int64_t
bim::axmol::app::coins_balance(const iscool::preferences::local_preferences& p)
{
  return p.get_value("coins", (std::int64_t)0);
}

void bim::axmol::app::coins_balance(iscool::preferences::local_preferences& p,
                                    std::int64_t v)
{
  p.set_value("coins", v);
}

void bim::axmol::app::add_coins(iscool::preferences::local_preferences& p,
                                std::int64_t a)
{
  return p.set_value("coins", coins_balance(p) + a);
}

void bim::axmol::app::consume_coins(iscool::preferences::local_preferences& p,
                                    std::int64_t a)
{
  const std::int64_t b = coins_balance(p);
  assert(b >= a);
  p.set_value("coins", b - a);
}
