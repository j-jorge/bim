// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/display/device_size.hpp>

#include <axmol/base/Director.h>
#include <axmol/platform/GLView.h>

ax::Vec2 bim::axmol::display::device_size()
{
  ax::Director* const director = ax::Director::getInstance();
  ax::GLView* const view = director->getGLView();

  assert(view != nullptr);

  return view->getVisibleSize();
}
