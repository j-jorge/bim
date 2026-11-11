// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/font_catalog.hpp>

struct bim::axmol::widget::font_catalog::entry
{
  std::string name;
  bool force_italics;
};

bim::axmol::widget::font_catalog::font_catalog() = default;
bim::axmol::widget::font_catalog::~font_catalog() = default;

bim::axmol::widget::font_catalog::resolve_result
bim::axmol::widget::font_catalog::resolve(const std::string& name) const
{
  const alias_map::const_iterator it = m_alias.find(name);

  if (it != m_alias.end())
    return { .name = it->second.name,
             .force_italics = it->second.force_italics };

  return { .name = name, .force_italics = false };
}

void bim::axmol::widget::font_catalog::set_alias(const std::string& key,
                                                 const std::string& font,
                                                 bool force_italics)
{
  m_alias[key] = { .name = font, .force_italics = force_italics };
}
