// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/input/backend_event.hpp>
#include <bim/axmol/input/backend_event.impl.hpp>
#include <bim/axmol/input/backend_event_view.hpp>
#include <bim/axmol/input/backend_event_view.impl.hpp>
#include <bim/axmol/input/key_event_view.hpp>
#include <bim/axmol/input/key_observer_handle.hpp>
#include <bim/axmol/input/touch_event_view.hpp>
#include <bim/axmol/input/touch_observer_handle.hpp>

#include <bim/axmol/input/observer/scroll_view_glue.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>
#include <bim/axmol/input/observer/tap_observer.hpp>

template class bim::axmol::input::backend_event<ax::EventKeyboard::KeyCode>;
template class bim::axmol::input::backend_event<ax::Touch*>;

template class bim::axmol::input::backend_event_view<
    bim::axmol::input::key_event>;
template class bim::axmol::input::backend_event_view<
    bim::axmol::input::touch_event>;

template class bim::axmol::input::key_observer_handle<
    bim::axmol::input::single_key_observer>;
template class bim::axmol::input::touch_observer_handle<
    bim::axmol::input::tap_observer>;
template class bim::axmol::input::touch_observer_handle<
    bim::axmol::input::scroll_view_glue>;
