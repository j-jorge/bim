// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/application_event_dispatcher.hpp>

#include <iscool/signals/signal.impl.tpp>

bim::axmol::app::application_event_dispatcher::application_event_dispatcher() =
    default;
bim::axmol::app::application_event_dispatcher::
    ~application_event_dispatcher() = default;

void bim::axmol::app::application_event_dispatcher::dispatch(
    std::string_view name) const
{
#if !__ANDROID__
  m_signal(name);
#endif
}
