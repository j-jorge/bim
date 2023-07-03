#pragma once

#include <bim/axmol/ref_ptr.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <string>

namespace ax
{
  class Node;
}

namespace bim::axmol::widget
{
  using named_node_group =
      boost::unordered_flat_map<std::string, bim::axmol::ref_ptr<ax::Node>>;
}
