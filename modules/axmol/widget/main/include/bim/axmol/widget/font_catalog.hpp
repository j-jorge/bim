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
      struct resolve_result
      {
        const std::string& name;
        bool force_italics;
      };

    public:
      font_catalog();
      ~font_catalog();

      resolve_result resolve(const std::string& name) const;

      void set_alias(const std::string& key, const std::string& font,
                     bool force_italics);

    private:
      struct entry;

    private:
      using alias_map = boost::unordered_flat_map<std::string, entry>;

    private:
      alias_map m_alias;
    };
  }
}
