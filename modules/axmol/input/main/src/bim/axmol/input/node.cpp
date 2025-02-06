// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/node.hpp>

#include <bim/axmol/input/key_observer.hpp>
#include <bim/axmol/input/node_reference.hpp>
#include <bim/axmol/input/touch_event_view.hpp>
#include <bim/axmol/input/touch_observer.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

static std::size_t g_next_node_id = 0;

bim::axmol::input::node::node(touch_observer_pointer touch,
                              key_observer_pointer key)
  : m_id(g_next_node_id++)
  , m_touch_observer(std::move(touch))
  , m_key_observer(std::move(key))
  , m_selected_in_scan(false)
{}

bim::axmol::input::node::node(key_observer_pointer instance)
  : node(touch_observer_pointer(), std::move(instance))
{}

void bim::axmol::input::node::attach(touch_observer_pointer observer)
{
  assert(observer.lock());
  m_touch_observer = std::move(observer);
}

void bim::axmol::input::node::attach(key_observer_pointer observer)
{
  assert(observer.lock());
  m_key_observer = std::move(observer);
}

void bim::axmol::input::node::push_back(touch_observer_pointer observer)
{
  assert(observer.lock());
  push_back(std::make_shared<node>(std::move(observer)));
}

void bim::axmol::input::node::push_back(key_observer_pointer observer)
{
  assert(observer.lock());
  push_back(std::make_shared<node>(std::move(observer)));
}

void bim::axmol::input::node::push_back(const node_reference& child)
{
  assert(child.m_node != nullptr);
  m_children.push_back(child.m_node);
  assert(check_no_duplicates());
}

void bim::axmol::input::node::pop_back()
{
  m_children.pop_back();
}

void bim::axmol::input::node::erase(const node_reference& child)
{
  const children_vector::iterator it(
      std::ranges::find(m_children, child.m_node));

  if (it != m_children.end())
    m_children.erase(it);
}

void bim::axmol::input::node::erase(const touch_observer_pointer& observer)
{
  assert(!observer.expired());

  const std::shared_ptr<touch_observer> pointer(observer.lock());
  const auto predicate = [&pointer](const node_pointer& p) -> bool
  {
    return p->m_touch_observer.lock() == pointer;
  };

  const children_vector::iterator it(
      std::ranges::find_if(m_children, predicate));

  assert(it != m_children.end());
  m_children.erase(it);
}

void bim::axmol::input::node::erase(const key_observer_pointer& observer)
{
  assert(!observer.expired());

  const std::shared_ptr<key_observer> pointer(observer.lock());
  const auto predicate = [&pointer](const node_pointer& p) -> bool
  {
    return p->m_key_observer.lock() == pointer;
  };

  const children_vector::iterator it(
      std::ranges::find_if(m_children, predicate));

  assert(it != m_children.end());
  m_children.erase(it);
}

void bim::axmol::input::node::clear()
{
  m_selected_in_scan = false;

  m_touch_observer.reset();
  m_key_observer.reset();
  m_children.clear();
}

void bim::axmol::input::node::touch_pressed(const touch_event_view& touches)
{
  auto visit(
      [&touches](node& n) -> void
      {
        const std::shared_ptr<touch_observer> observer(
            n.m_touch_observer.lock());

        if (observer != nullptr)
          observer->pressed(touches);
      });

  depth_first_scan(visit);
}

void bim::axmol::input::node::touch_moved(const touch_event_view& touches)
{
  auto visit(
      [&touches](const node& n) -> void
      {
        const std::shared_ptr<touch_observer> observer(
            n.m_touch_observer.lock());

        if (observer != nullptr)
          observer->moved(touches);
      });

  depth_first_scan(visit);
}

void bim::axmol::input::node::touch_released(const touch_event_view& touches)
{
  auto visit(
      [&touches](node& n) -> void
      {
        const std::shared_ptr<touch_observer> observer(
            n.m_touch_observer.lock());

        if (observer != nullptr)
          observer->released(touches);
      });

  depth_first_scan(visit);
}

void bim::axmol::input::node::touch_cancelled(const touch_event_view& touches)
{
  auto visit(
      [&touches](node& n) -> void
      {
        const std::shared_ptr<touch_observer> observer(
            n.m_touch_observer.lock());

        if (observer != nullptr)
          observer->cancelled(touches);
      });

  depth_first_scan(visit);
}

void bim::axmol::input::node::key_pressed(const key_event_view& keys)
{
  auto visit(
      [&keys](node& n) -> void
      {
        const std::shared_ptr<key_observer> observer(n.m_key_observer.lock());

        if (observer != nullptr)
          observer->pressed(keys);
      });

  depth_first_scan(visit);
}

void bim::axmol::input::node::key_released(const key_event_view& keys)
{
  auto visit(
      [&keys](node& n) -> void
      {
        const std::shared_ptr<key_observer> observer(n.m_key_observer.lock());

        if (observer != nullptr)
          observer->released(keys);
      });

  depth_first_scan(visit);
}

bool bim::axmol::input::node::check_no_duplicates() const
{
  std::unordered_set<touch_observer*> touch_nodes;
  std::unordered_set<key_observer*> key_nodes;

  return check_no_duplicate_touch_observer(touch_nodes)
         && check_no_duplicate_key_observer(key_nodes);
}

std::string bim::axmol::input::node::to_string() const
{
  std::ostringstream stream;
  to_string(stream, 0);
  return stream.str();
}

bool bim::axmol::input::node::check_no_duplicate_touch_observer(
    std::unordered_set<touch_observer*>& result) const
{
  if (!m_touch_observer.expired())
    {
      const auto test(result.insert(m_touch_observer.lock().get()));

      if (!test.second)
        return false;
    }

  for (const node_pointer& child : m_children)
    if (!child->check_no_duplicate_touch_observer(result))
      return false;

  return true;
}

bool bim::axmol::input::node::check_no_duplicate_key_observer(
    std::unordered_set<key_observer*>& result) const
{
  if (!m_key_observer.expired())
    {
      const auto test(result.insert(m_key_observer.lock().get()));

      if (!test.second)
        return false;
    }

  for (const node_pointer& child : m_children)
    if (!child->check_no_duplicate_key_observer(result))
      return false;

  return true;
}

void bim::axmol::input::node::to_string(std::ostream& stream,
                                        std::size_t indentation) const
{
  std::fill_n(std::ostream_iterator<char>(stream), indentation, '-');
  stream << "> [" << m_id << "] touch: ";

  if (m_touch_observer.expired())
    stream << "null";
  else
    {
      touch_observer* const touch = m_touch_observer.lock().get();
      stream << '(' << typeid(*touch).name() << ':' << touch << ')';
    }

  stream << ", key: ";

  if (m_key_observer.expired())
    stream << "null";
  else
    {
      key_observer* const key = m_key_observer.lock().get();
      stream << '(' << typeid(*key).name() << ':' << key << ')';
    }

  stream << '\n';

  for (const node_pointer& child : m_children)
    child->to_string(stream, indentation + 1);
}

template <typename Visit>
void bim::axmol::input::node::depth_first_scan(Visit&& visit)
{
  assert(!m_selected_in_scan);
  m_selected_in_scan = true;

  visit_selected(depth_first_select(*this), visit);

  if (!m_selected_in_scan)
    return;

  visit(*this);
  m_selected_in_scan = false;
}

bim::axmol::input::node::children_vector
bim::axmol::input::node::depth_first_select(const node& root)
{
  static constexpr std::size_t queue_capacity(2048);

  children_vector pending;
  pending.reserve(queue_capacity);
  pending.insert(pending.end(), root.m_children.begin(),
                 root.m_children.end());

  children_vector selected;
  selected.reserve(queue_capacity);

  while (!pending.empty())
    {
      const typename children_vector::value_type child(pending.back());
      pending.pop_back();

      assert(!child->m_selected_in_scan);
      child->m_selected_in_scan = true;

      selected.push_back(child);
      pending.insert(pending.end(), child->m_children.begin(),
                     child->m_children.end());
    }

  return selected;
}

template <typename Visit>
void bim::axmol::input::node::visit_selected(const children_vector& selected,
                                             Visit&& visit)
{
  for (children_vector::const_reverse_iterator it = selected.rbegin(),
                                               eit = selected.rend();
       it != eit; ++it)
    if ((*it)->m_selected_in_scan)
      {
        visit(**it);
        (*it)->m_selected_in_scan = false;
      }
}
