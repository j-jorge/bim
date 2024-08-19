// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/tree.hpp>

#include <bim/axmol/input/node.hpp>

bim::axmol::input::tree::tree()
  : m_root(new node())
{}

bim::axmol::input::node_reference bim::axmol::input::tree::root() const
{
  return m_root;
}

std::string bim::axmol::input::tree::to_string() const
{
  return m_root->to_string();
}

void bim::axmol::input::tree::push_back(const touch_observer_pointer& observer)
{
  m_root->push_back(observer);
}

void bim::axmol::input::tree::push_back(const key_observer_pointer& observer)
{
  m_root->push_back(observer);
}

void bim::axmol::input::tree::push_back(const node_reference& child)
{
  m_root->push_back(child);
}

void bim::axmol::input::tree::erase(const node_reference& child)
{
  m_root->erase(child);
}

void bim::axmol::input::tree::clear()
{
  m_root->clear();
}

void bim::axmol::input::tree::attach_to_root(
    const touch_observer_pointer& observer)
{
  m_root->attach(observer);
}

void bim::axmol::input::tree::attach_to_root(
    const key_observer_pointer& observer)
{
  m_root->attach(observer);
}
