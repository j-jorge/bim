// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <iscool/signals/connection.hpp>

#include <functional>
#include <string_view>

namespace bim::axmol::app
{
  class application_event_dispatcher;

  class application_event_listener
  {
  public:
    explicit application_event_listener(
        application_event_dispatcher& dispatcher);

    iscool::signals::connection
    connect_to_event(std::function<void(std::string_view)> f) const;

  private:
    application_event_dispatcher& m_dispatcher;
  };
}
