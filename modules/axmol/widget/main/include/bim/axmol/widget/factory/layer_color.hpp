#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class LayerColor;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::LayerColor from a
   * style.
   */
  template <>
  class factory<ax::LayerColor>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::LayerColor>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
