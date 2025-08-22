// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class ProgressTimer;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::ProgressTimer
   * from a style.
   */
  template <>
  class factory<ax::ProgressTimer>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ProgressTimer>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
