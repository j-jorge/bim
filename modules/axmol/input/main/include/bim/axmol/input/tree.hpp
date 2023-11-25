#pragma once

#include <bim/axmol/input/key_observer_pointer.hpp>
#include <bim/axmol/input/node_reference.hpp>
#include <bim/axmol/input/touch_observer_pointer.hpp>

#include <string>

namespace bim::axmol::input
{
  class tree
  {
  public:
    tree();

    node_reference root() const;
    std::string to_string() const;

    void push_back(const touch_observer_pointer& observer);
    void push_back(const key_observer_pointer& observer);
    void push_back(const node_reference& child);

    void erase(const node_reference& child);
    void clear();
    void attach_to_root(const touch_observer_pointer& observer);
    void attach_to_root(const key_observer_pointer& observer);

  private:
    node_pointer m_root;
  };
}
