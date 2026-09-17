// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/input/tree.hpp>

#include <iscool/context.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <memory>
#include <string_view>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::app
{
  class analytics_service;
  class config;
  class player_profile;
  class update_nickname_job;
}

namespace bim::business
{
  class request_headers;
}

namespace bim::axmol::app
{
  class application_event_dispatcher;
  class main_scene;
  class message_popup;
  class popup;

  class nickname_editor_popup
  {
    DECLARE_VOID_SIGNAL(closed, m_closed)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                          //
            ((const bim::business::request_headers*)(request_headers)) //
            ((bim::app::player_profile*)(player_profile))              //
            ((bim::app::analytics_service*)(analytics))                //
            ((const bim::app::config*)(config))                        //
            ((const bim::axmol::widget::context*)(widget_context))     //
            ((main_scene*)(main_scene))                                //
            ((application_event_dispatcher*)(event_dispatcher))        //
            ),
        ic_context_no_properties);

  public:
    nickname_editor_popup(const context& context,
                          const iscool::style::declaration& style);
    ~nickname_editor_popup();

    void show();

  private:
    void reset_display();
    void display_error();
    void enable(bool enabled);
    void check_new_nickname();

    void validate();
    void start_job();
    void close();

  private:
    bim_declare_controls_struct(controls, m_controls, 4);
    const iscool::style::declaration& m_style_bounds;

    std::unique_ptr<popup> m_popup;
    std::unique_ptr<message_popup> m_message_popup;
    std::unique_ptr<bim::app::update_nickname_job> m_job;

    bim::axmol::input::single_key_observer_handle m_escape;
    bim::axmol::input::tree m_inputs;

    iscool::signals::connection m_done_connection;
    iscool::signals::connection m_error_connection;

    std::string m_clean_nickname;
  };
}
