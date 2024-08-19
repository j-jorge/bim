// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

template <typename Event>
bim::axmol::input::backend_event_view<Event>::backend_event_view(
    event_span span)
  : m_events(span)
{}

template <typename Event>
void bim::axmol::input::backend_event_view<Event>::consume_all() const
{
  for (Event& e : m_events)
    if (e.is_available())
      e.consume();
}

template <typename Event>
bool bim::axmol::input::backend_event_view<Event>::is_fully_consumed() const
{
  for (const Event& e : m_events)
    if (e.is_available())
      return false;

  return true;
}

template <typename Event>
typename bim::axmol::input::backend_event_view<Event>::iterator
bim::axmol::input::backend_event_view<Event>::begin() const
{
  return m_events.begin();
}

template <typename Event>
typename bim::axmol::input::backend_event_view<Event>::iterator
bim::axmol::input::backend_event_view<Event>::end() const
{
  return m_events.end();
}

template <typename Event>
std::size_t bim::axmol::input::backend_event_view<Event>::size() const
{
  return m_events.size();
}

template <typename Event>
bool bim::axmol::input::backend_event_view<Event>::empty() const
{
  return m_events.empty();
}
