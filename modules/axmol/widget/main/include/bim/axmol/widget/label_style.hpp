// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <axmol/2d/Label.h>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  class label_style
  {
  public:
    label_style(const bim::axmol::widget::context& context,
                const iscool::style::declaration& style);

    ax::TTFConfig ttf_config;
    ax::Color4B color;
    ax::TextVAlignment vertical_align;
    ax::TextHAlignment horizontal_align;
    std::string text;
  };
}
