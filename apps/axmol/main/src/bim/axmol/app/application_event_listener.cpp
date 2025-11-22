// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/application_event_listener.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>

#include <iscool/signals/signal.impl.tpp>

bim::axmol::app::application_event_listener::application_event_listener(
    application_event_dispatcher& dispatcher)
  : m_dispatcher(dispatcher)
{}

iscool::signals::connection
bim::axmol::app::application_event_listener::connect_to_event(
    std::function<void(std::string_view)> f) const
{
  return m_dispatcher.m_signal.connect(std::move(f));
}
