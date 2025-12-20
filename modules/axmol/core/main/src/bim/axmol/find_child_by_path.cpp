// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/find_child_by_path.hpp>

#include <axmol/2d/Node.h>

static ax::Node* find_child(ax::Node& node, std::string_view path,
                            std::string_view::size_type p)
{
  const std::string_view::size_type e = path.size();

  const std::string_view::size_type s =
      std::min(e, path.find_first_of('/', p));
  const std::string_view name = path.substr(p, s - p);
  const std::uint64_t hash = ax::hashNodeName(name);

  for (ax::Node* n : node.getChildren())
    if ((n->getHashOfName() == hash) && (n->getName() == name))
      {
        if (s + 1 >= e)
          return n;

        ax::Node* c = find_child(*n, path, s + 1);

        if (c)
          return c;
      }

  return nullptr;
}

ax::Node* bim::axmol::find_child_by_path(ax::Node& node, std::string_view path)
{
  if (path.empty())
    return &node;

  return find_child(node, path, 0);
}
