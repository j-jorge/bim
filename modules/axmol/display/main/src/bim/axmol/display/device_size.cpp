// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/display/device_size.hpp>

#include <bim/assert.hpp>

#include <axmol/base/Director.h>
#include <axmol/platform/RenderView.h>

ax::Vec2 bim::axmol::display::device_size()
{
  ax::Director* const director = ax::Director::getInstance();
  const ax::RenderView* const view = director->getRenderView();

  bim_assert(view != nullptr);

  return view->getVisibleSize();
}
