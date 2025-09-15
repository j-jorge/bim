// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax::ui
{
  class RichText;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::ui::RichText from
   * a style.
   */
  template <>
  class factory<ax::ui::RichText>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ui::RichText>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
