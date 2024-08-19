// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/factory.hpp>

namespace ax
{
  class Node;
}

namespace bim::axmol::widget
{
  /**
   * Specialization of the factory to be able to build an ax::Node from a
   * style.
   */
  template <>
  class factory<ax::Node>
  {
  public:
    [[nodiscard]] static bim::axmol::ref_ptr<ax::Node>
    create(const bim::axmol::widget::context& context,
           const iscool::style::declaration&);
  };
};
