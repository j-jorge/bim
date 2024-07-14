#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class ClippingNode;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::ClippingNode from
   * a style.
   */
  template <>
  class factory<ax::ClippingNode>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::ClippingNode>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
