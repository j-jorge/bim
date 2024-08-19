// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/factory/dynamic_factory.hpp>

namespace ax
{
  class FiniteTimeAction;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol
{
  class colour_chart;

  namespace action
  {
    /**
     * This class provides a way to create an instance of a class derived from
     * ax::FiniteTimeAction given its type name.
     */
    class dynamic_factory
    {
    public:
      template <typename F>
      void register_action(const std::string& name, F&& f);

      [[nodiscard]] bim::axmol::ref_ptr<ax::FiniteTimeAction>
      create(const bim::axmol::colour_chart& colors,
             const iscool::style::declaration& style) const;

    private:
      iscool::factory::dynamic_factory<
          bim::axmol::ref_ptr<ax::FiniteTimeAction>,
          const bim::axmol::colour_chart&, const iscool::style::declaration&>
          m_factory;
    };
  }
}
