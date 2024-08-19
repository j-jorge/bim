// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/key_event_view.hpp>
#include <bim/axmol/input/key_observer_pointer.hpp>
#include <bim/axmol/input/node_pointer.hpp>
#include <bim/axmol/input/touch_event_view.hpp>
#include <bim/axmol/input/touch_observer_pointer.hpp>

#include <unordered_set>
#include <vector>

namespace bim::axmol::input
{
  class node_reference;

  /**
   * The node handles the input tree as well as the event propagation. It uses
   * shared and weak pointers to keep track of the availability of the
   * observers in case where they would be removed during the scan of the
   * tree. This is a poor mechanism but it is easy to implement.
   */
  class node
  {
  public:
    explicit node(touch_observer_pointer touch = touch_observer_pointer(),
                  key_observer_pointer key = key_observer_pointer());
    explicit node(key_observer_pointer instance);

    node(const node&) = delete;
    node& operator=(const node&) = delete;

    void attach(touch_observer_pointer observer);
    void attach(key_observer_pointer observer);

    void push_back(touch_observer_pointer observer);
    void push_back(key_observer_pointer observer);
    void push_back(const node_reference& child);

    void erase(const node_reference& child);
    void erase(const touch_observer_pointer& observer);
    void erase(const key_observer_pointer& observer);

    void clear();

    void touch_pressed(const touch_event_view& touches);
    void touch_moved(const touch_event_view& touches);
    void touch_released(const touch_event_view& touches);
    void touch_cancelled(const touch_event_view& touches);

    void key_pressed(const key_event_view& keys);
    void key_released(const key_event_view& keys);

    std::string to_string() const;
    bool check_no_duplicates() const;

  private:
    using children_vector = std::vector<node_pointer>;

  private:
    bool check_no_duplicate_touch_observer(
        std::unordered_set<touch_observer*>& result) const;

    bool check_no_duplicate_key_observer(
        std::unordered_set<key_observer*>& result) const;

    void to_string(std::ostream& stream, std::size_t indentation) const;

    template <typename Visit>
    void depth_first_scan(Visit&& visit);

    static children_vector depth_first_select(const node& root);

    template <typename Visit>
    static void visit_selected(const children_vector& selected, Visit&& visit);

  private:
    const std::size_t m_id;

    children_vector m_children;

    touch_observer_pointer m_touch_observer;
    key_observer_pointer m_key_observer;

    bool m_selected_in_scan;
  };
}
