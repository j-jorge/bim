// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <boost/unordered/unordered_flat_map.hpp>

namespace bim::axmol
{
  namespace widget
  {
    class font_catalog
    {
    public:
      const std::string& resolve(const std::string& name) const;

      void set_alias(const std::string& key, const std::string& font);

    private:
      using alias_map = boost::unordered_flat_map<std::string, std::string>;

    private:
      alias_map m_alias;
    };
  }
}
