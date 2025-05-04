// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/find_child_by_path.hpp>

#include <axmol/2d/Node.h>

ax::Node* bim::axmol::find_child_by_path(ax::Node& node, std::string_view path)
{
  const std::string_view::size_type e = path.size();
  ax::Node* n = &node;
  std::string_view::size_type s = 0;
  std::string_view::size_type p = std::min(e, path.find_first_of('/'));

  while ((n != nullptr) && (s < e))
    {
      n = n->getChildByName(path.substr(s, p - s));
      s = p + 1;
      p = std::min(e, path.find_first_of('/', s));
    }

  return n;
}
