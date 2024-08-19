// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax::ui
{
  class Scale9Sprite;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::ui::Scale9Sprite
   * from a style.
   */
  template <>
  class factory<ax::ui::Scale9Sprite>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ui::Scale9Sprite>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
