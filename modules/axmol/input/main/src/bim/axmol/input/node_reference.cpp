// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/node_reference.hpp>

#include <cassert>
#include <utility>

bim::axmol::input::node_reference::node_reference(node_pointer node)
  : m_node(std::move(node))
{
  assert(m_node != nullptr);
}
