// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class ClippingRectangleNode;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an
   * ax::ClippingRectangleNode from a style.
   */
  template <>
  class factory<ax::ClippingRectangleNode>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ClippingRectangleNode>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
