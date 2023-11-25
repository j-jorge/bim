#pragma once

#include <bim/axmol/input/node_pointer.hpp>

namespace bim::axmol::input
{
  class node_reference
  {
    friend class node;

  public:
    node_reference(node_pointer node);

  private:
    node_pointer m_node;
  };
}
