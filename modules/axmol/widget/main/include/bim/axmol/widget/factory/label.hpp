#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class Label;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::Label from a
   * style.
   */
  template <>
  class factory<ax::Label>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::Label>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
