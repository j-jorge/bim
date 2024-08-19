// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/factory/dynamic_factory.hpp>

namespace ax
{
  class Node;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;

  /**
   * This class provides a way to create an instance of a class derived from
   * ax::Node given its type name.
   */
  class dynamic_factory
  {
  public:
    template <typename T>
    void register_widget(const std::string& type);

    [[nodiscard]] bim::axmol::ref_ptr<ax::Node>
    create(std::string_view type, const context& c,
           const iscool::style::declaration& style) const;

  private:
    iscool::factory::dynamic_factory<bim::axmol::ref_ptr<ax::Node>,
                                     const context&,
                                     const iscool::style::declaration&>
        m_factory;
  };
};
