// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/display/main_view.hpp>

#include <axmol/base/Director.h>

#ifdef __ANDROID__
  #include <axmol/platform/android/RenderViewImpl-android.h>
#else
  #include <axmol/platform/RenderViewImpl.h>
#endif

bim::axmol::display::main_view::main_view(std::string_view title,
                                          const ax::Size& size, float scale)
{
  ax::Director* const director(ax::Director::getInstance());

  ax::RenderView* view = director->getRenderView();

  if (view != nullptr)
    return;

  const ax::Rect view_rect(0, 0, size.width * scale, size.height * scale);
  view = ax::RenderViewImpl::createWithRect(title, view_rect);

  director->setRenderView(view);
  view->setDesignResolutionSize(size.width, size.height,
                                ResolutionPolicy::SHOW_ALL);

  GfxContextAttrs gfx_context_attrs = { .redBits = 8,
                                        .greenBits = 8,
                                        .blueBits = 8,
                                        .alphaBits = 8,
                                        .depthBits = 24,
                                        .stencilBits = 8,
                                        .multisamplingCount = 0 };
  ax::RenderView::setGfxContextAttrs(gfx_context_attrs);
}

bim::axmol::display::main_view::~main_view() = default;
