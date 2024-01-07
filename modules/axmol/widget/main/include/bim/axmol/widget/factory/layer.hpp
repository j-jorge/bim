#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class Layer;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::Layer from a
   * style.
   */
  template <>
  class factory<ax::Layer>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::Layer>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
