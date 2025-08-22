// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/progress_timer.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/ProgressTimer.h>
#include <axmol/2d/Sprite.h>

bim::axmol::ref_ptr<ax::ProgressTimer>
bim::axmol::widget::factory<ax::ProgressTimer>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const bim::axmol::ref_ptr<ax::Sprite> sprite = factory<ax::Sprite>::create(
      context, style.get_declaration_or_empty("sprite"));

  ax::ProgressTimer* const result = ax::ProgressTimer::create(sprite.get());

  const std::string type = style.get_string("type", "bar");

  if (type == "bar")
    result->setType(ax::ProgressTimer::Type::BAR);
  else if (type == "radial")
    result->setType(ax::ProgressTimer::Type::RADIAL);
  else
    ic_log(iscool::log::nature::error(), g_log_context,
           "Unknown progress timer type '{}'.", type);

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
