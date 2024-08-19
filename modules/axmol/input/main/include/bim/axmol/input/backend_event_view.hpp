// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/backend_event.hpp>

#include <span>

namespace bim::axmol::input
{
  template <typename Event>
  class backend_event_view
  {
  private:
    using event_span = std::span<Event>;

  public:
    using iterator = typename event_span::iterator;

  public:
    explicit backend_event_view(event_span span);

    void consume_all() const;
    bool is_fully_consumed() const;

    iterator begin() const;
    iterator end() const;

    std::size_t size() const;
    bool empty() const;

  private:
    event_span m_events;
  };
}
