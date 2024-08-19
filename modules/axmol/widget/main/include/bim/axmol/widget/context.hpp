// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/context.hpp>

namespace bim::axmol
{
  class colour_chart;

  namespace action
  {
    class dynamic_factory;
  }

  namespace style
  {
    class cache;
  }

  namespace widget
  {
    class dynamic_factory;

    class context
    {
    public:
      const bim::axmol::colour_chart& colors;
      bim::axmol::style::cache& style_cache;
      const bim::axmol::widget::dynamic_factory& factory;
      const bim::axmol::action::dynamic_factory& action_factory;

      /**
       * Scale factor to apply to the widgets to stretch them from the design
       * space to the device space.
       */
      float device_scale;
    };
  }
}
