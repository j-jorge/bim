// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/font_catalog.hpp>

const std::string&
bim::axmol::widget::font_catalog::resolve(const std::string& name) const
{
  const alias_map::const_iterator it = m_alias.find(name);

  if (it != m_alias.end())
    return it->second;

  return name;
}

void bim::axmol::widget::font_catalog::set_alias(const std::string& key,
                                                 const std::string& font)
{
  m_alias[key] = font;
}
