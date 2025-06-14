// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/action/dynamic_factory.hpp>
#include <bim/axmol/audio/mixer.hpp>
#include <bim/axmol/colour_chart.hpp>
#include <bim/axmol/display/main_view.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/observer/single_key_observer_handle.hpp>
#include <bim/axmol/style/cache.hpp>
#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/dynamic_factory.hpp>

#include <iscool/signals/scoped_connection.hpp>

#include <iscool/context.hpp>
#include <iscool/schedule/worker.hpp>

#include <axmol/platform/Application.h>

#include <string>
#include <vector>

namespace iscool
{
  namespace audio
  {
    class mixer;
  }

  namespace preferences
  {
    class local_preferences;
  }

  namespace social
  {
    class service;
  }

  namespace system
  {
    class haptic_feedback;
  }
}

namespace bim::axmol::app
{
  namespace detail
  {
    /// Those are the systems that survive a reset of the game.
    struct persistent_systems;

    /// Those are the systems that must be restarted on a reset of the game.
    struct session_systems;
  }

  class main_scene;
  class main_task;
  class scene_lock;

  class application : private ax::Application
  {
    friend struct detail::persistent_systems;
    friend struct detail::session_systems;

    ic_declare_context(
        m_context, ic_context_no_parent_properties,
        ic_context_declare_properties(                                     //
            ((bim::axmol::widget::context)(widget_context))                //
            ((main_scene*)(main_scene))                                    //
            ((scene_lock*)(scene_lock))                                    //
            ((iscool::audio::mixer*)(audio))                               //
            ((iscool::preferences::local_preferences*)(local_preferences)) //
            ((iscool::social::service*)(social))                           //
            ((iscool::system::haptic_feedback*)(haptic_feedback))          //
            ((bool)(enable_debug))));

  public:
    application();
    application(std::vector<std::string> asset_directories,
                const ax::Size& screen_size, float screen_scale,
                bool enable_debug);
    ~application();

    bool applicationDidFinishLaunching() override;
    void applicationDidEnterBackground() override;
    void applicationWillEnterForeground() override;

  private:
    struct screen_config
    {
      ax::Size size;
      float scale;
    };

  private:
    void complete_launch();

    void clean_up();
    void reset();

    void set_up_file_utils();
    void set_up_colour_chart();

    void set_up_local_preferences();
    void tear_down_local_preferences();
    void flush_local_preferences();
    void apply_local_preferences();

    void launch_game();
    void stop_game();

    void listen_to_frame_event();
    void tick();

    void capture_scren() const;

  private:
    std::vector<std::string> m_asset_directories;

    iscool::signals::scoped_connection m_launch_connection;

    std::unique_ptr<bim::axmol::display::main_view> m_main_view;

    bim::axmol::colour_chart m_colors;
    bim::axmol::style::cache m_style_cache;
    bim::axmol::action::dynamic_factory m_action_factory;
    bim::axmol::widget::dynamic_factory m_widget_factory;

    std::unique_ptr<detail::persistent_systems> m_persistent_systems;
    std::unique_ptr<detail::session_systems> m_session_systems;

    std::unique_ptr<main_task> m_main_task;

    bim::axmol::input::single_key_observer_handle
        m_screen_capture_key_observer;
    bim::axmol::input::single_key_observer_handle m_reset_key_observer;
    bim::axmol::input::node m_input_root;

    screen_config m_screen_config;
  };
}
