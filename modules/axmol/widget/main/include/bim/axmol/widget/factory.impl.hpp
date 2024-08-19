// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

template <typename T>
bim::axmol::ref_ptr<T>
bim::axmol::widget::factory<T>::create(const context& c,
                                       const iscool::style::declaration& style)
{
  const bim::axmol::ref_ptr<T> result(T::create(c, style));

  bim::axmol::style::apply_display(c.style_cache.get_display(style), *result);

  return result;
}
