// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/action/runner.hpp>
#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>
#include <iscool/schedule/scoped_connection.hpp>
#include <iscool/signals/declare_signal.hpp>

#include <json/value.h>

namespace iscool::preferences
{
  class local_preferences;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::widget
{
  class context;
}

namespace bim::axmol::app
{
  class main_scene;

  class loading_screen
  {
    DECLARE_VOID_SIGNAL(done, m_done)

    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                              //
            ((const bim::axmol::widget::context&)(widget_context))         //
            ((main_scene*)(main_scene))                                    //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ),
        ic_context_no_properties);

  public:
    loading_screen(const context& context,
                   const iscool::style::declaration& style);
    ~loading_screen();

    void start();
    void stop();

  private:
    void load_resources();

    void load_translations();
    void load_textures();
    void load_sprite_sheets();
    void load_shaders();
    void load_styles();

    void one_loaded_texture();
    void one_loaded_resource();

    void stopped();

  private:
    bim::axmol::ref_ptr<ax::Node> m_container;
    bim::axmol::input::tree m_inputs;

    bim_declare_controls_struct(controls, m_controls, 0);

    const iscool::style::declaration& m_bounds;
    const iscool::style::declaration& m_action_done;

    bim::axmol::action::runner m_action_runner;

    iscool::schedule::scoped_connection m_start_connection;

    Json::Value m_resources;
    std::uint8_t m_pending_resources;
    std::uint8_t m_pending_textures;
  };
}
