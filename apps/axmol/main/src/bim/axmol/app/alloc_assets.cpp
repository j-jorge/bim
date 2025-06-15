// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/alloc_assets.hpp>

#include <bim/axmol/widget/factory/sprite.hpp>

#include <axmol/2d/Node.h>
#include <axmol/2d/Sprite.h>

void bim::axmol::app::alloc_assets(std::vector<ax::Sprite*>& out,
                                   const bim::axmol::widget::context& context,
                                   std::size_t count,
                                   const iscool::style::declaration& style,
                                   ax::Node& parent)
{
  out.resize(count);

  for (ax::Sprite*& p : out)
    p = bim::axmol::widget::factory<ax::Sprite>::create(context, style).get();

  for (ax::Sprite* p : out)
    parent.addChild(p);
}
