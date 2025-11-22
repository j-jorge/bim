// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/signals/signal.hpp>

#include <string_view>

namespace bim::axmol::app
{
  class application_event_listener;

  class application_event_dispatcher
  {
    friend class application_event_listener;

  public:
    application_event_dispatcher();
    ~application_event_dispatcher();

    void dispatch(std::string_view name) const;

  private:
    iscool::signals::signal<void(std::string_view)> m_signal;
  };
}
