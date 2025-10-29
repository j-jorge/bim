// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <iscool/strings/hash.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string>

namespace ax
{
  class Node;
}

namespace bim::axmol::widget
{
  using named_node_group =
      boost::unordered_flat_map<std::string, bim::axmol::ref_ptr<ax::Node>,
                                iscool::strings::hash, std::equal_to<>>;
}
