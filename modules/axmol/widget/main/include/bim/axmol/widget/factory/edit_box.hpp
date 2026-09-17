// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax::ui
{
  class EditBox;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::ui::EditBox from
   * a style.
   */
  template <>
  class factory<ax::ui::EditBox>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ui::EditBox>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
