// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/application.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>
#include <bim/axmol/app/application_event_listener.hpp>
#include <bim/axmol/app/main_scene.hpp>
#include <bim/axmol/app/root_scene.hpp>
#include <bim/axmol/app/script_director.hpp>
#include <bim/axmol/app/script_info.hpp>
#include <bim/axmol/app/task/main_task.hpp>
#include <bim/axmol/app/widget/register_widgets.hpp>

#include <bim/axmol/action/register_actions.hpp>
#include <bim/axmol/audio/mixer.hpp>
#include <bim/axmol/display/device_scale.hpp>
#include <bim/axmol/input/flow.hpp>
#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>
#include <bim/axmol/widget/register_widgets.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <bim/app/analytics_service.hpp>
#include <bim/app/preference/audio.hpp>
#include <bim/app/preference/haptic.hpp>

#include <iscool/audio/default_mixer.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/file_exists.hpp>
#include <iscool/files/get_writable_path.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/log/add_file_sink.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/setup.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/social/service.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/style/setup.hpp>
#include <iscool/system/device_date.hpp>
#include <iscool/system/haptic_feedback.hpp>
#include <iscool/system/language_name.hpp>
#include <iscool/time/now.hpp>

#include <axmol/2d/FontFreeType.h>
#include <axmol/2d/Scene.h>
#include <axmol/2d/SpriteFrameCache.h>
#include <axmol/base/Director.h>
#include <axmol/base/EventDispatcher.h>
#include <axmol/base/Utils.h>
#include <axmol/platform/FileUtils.h>
#include <axmol/platform/Image.h>
#include <axmol/renderer/TextureCache.h>
#include <axmol/renderer/backend/ProgramManager.h>

#include <ctime>
#include <iomanip>
#include <sstream>

static const char* const g_log_context = "StartUp";

struct bim::axmol::app::detail::persistent_systems
{
public:
  explicit persistent_systems(application& app);
  persistent_systems(const persistent_systems&) = delete;
  ~persistent_systems();

  bim::axmol::app::root_scene& root_scene();

private:
  void start_log_system();
  void stop_log_system();

  void start_display();
  void stop_display();

  void start_social();
  void stop_social();

  void start_haptic_feedback();
  void stop_haptic_feedback();

  void start_audio();
  void stop_audio();

  void listen_to_frame_event();
  void remove_frame_event_listener();

  void start_root_scene();
  void stop_root_scene();

private:
  application& m_application;
  bim::app::analytics_service m_analytics;
  bim::axmol::audio::mixer m_audio;
  bim::axmol::ref_ptr<bim::axmol::app::root_scene> m_root_scene;
  iscool::signals::scoped_connection m_clean_up_connection;
  application_event_dispatcher m_event_dispatcher;

  ax::EventListenerCustom* m_frame_event_listener;
};

struct bim::axmol::app::detail::session_systems
{
public:
  explicit session_systems(application& app);
  session_systems(const session_systems&) = delete;
  ~session_systems();

  const iscool::style::declaration& root_style() const;

private:
  void start_styles();
  void stop_styles();

  void start_main_scene();
  void stop_main_scene();

  void start_inputs();
  void stop_inputs();

private:
  application& m_application;

  std::unique_ptr<iscool::style::declaration> m_style;
  std::unique_ptr<bim::axmol::input::flow> m_input_flow;
};

bim::axmol::app::detail::persistent_systems::persistent_systems(
    application& app)
  : m_application(app)
{
  m_application.m_context.set_analytics(&m_analytics);
  m_application.m_context.set_event_dispatcher(&m_event_dispatcher);

  start_log_system();
  start_display();
  start_social();
  start_haptic_feedback();
  start_audio();
  listen_to_frame_event();
  start_root_scene();
}

bim::axmol::app::detail::persistent_systems::~persistent_systems()
{
  stop_root_scene();
  remove_frame_event_listener();
  stop_audio();
  stop_haptic_feedback();
  stop_social();
  stop_display();
  stop_log_system();
}

bim::axmol::app::root_scene&
bim::axmol::app::detail::persistent_systems::root_scene()
{
  return *m_root_scene;
}

void bim::axmol::app::detail::persistent_systems::start_log_system()
{
  iscool::log::initialize("Bim");

  const std::string log_file_path(iscool::files::get_writable_path()
                                  + "log.txt");

  iscool::log::add_file_sink(log_file_path);

  ic_log(iscool::log::nature::info(), g_log_context, "Log system initialized");

  std::tm now(iscool::system::device_date());
  char now_str[32];

  if (std::strftime(now_str, sizeof(now_str), "%FT%T%z", &now) == 0)
    ic_log(iscool::log::nature::info(), "StartUp", "Failed to get the date.");
  else
    ic_log(iscool::log::nature::info(), g_log_context, "{}",
           static_cast<const char*>(now_str));
}

void bim::axmol::app::detail::persistent_systems::stop_log_system()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: log system.");

  iscool::log::finalize();
}

void bim::axmol::app::detail::persistent_systems::start_display()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: display.");
  assert(m_application.m_main_view == nullptr);
  m_application.m_main_view.reset(new bim::axmol::display::main_view(
      "Bim!", m_application.m_screen_config.size,
      m_application.m_screen_config.scale));
}

void bim::axmol::app::detail::persistent_systems::stop_display()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: display.");

  m_application.m_main_view.reset();
}

void bim::axmol::app::detail::persistent_systems::start_haptic_feedback()
{
  ic_log(iscool::log::nature::info(), g_log_context,
         "Start: haptic feedback.");

  assert(!m_application.m_context.is_haptic_feedback_set());

  iscool::system::haptic_feedback* const feedback(
      new iscool::system::haptic_feedback());

  m_application.m_context.set_haptic_feedback(feedback);
}

void bim::axmol::app::detail::persistent_systems::stop_haptic_feedback()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: haptic feedback.");

  delete m_application.m_context.get_haptic_feedback();
  m_application.m_context.reset_haptic_feedback();
}

void bim::axmol::app::detail::persistent_systems::start_social()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: social.");

  assert(!m_application.m_context.is_social_set());

  iscool::social::service* const service(new iscool::social::service());

  m_application.m_context.set_social(service);
}

void bim::axmol::app::detail::persistent_systems::stop_social()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: social.");

  delete m_application.m_context.get_social();
  m_application.m_context.reset_social();
}

void bim::axmol::app::detail::persistent_systems::start_audio()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: audio.");

  iscool::audio::mixer* const mixer(new iscool::audio::mixer(
      iscool::resources::resolver("audio/", ".ogg"), 5, m_audio));

  iscool::audio::set_default_mixer(*mixer);
  m_application.m_context.set_audio(mixer);
}

void bim::axmol::app::detail::persistent_systems::stop_audio()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: audio.");

  iscool::audio::clear_default_mixer();
  delete m_application.m_context.get_audio();
  m_application.m_context.reset_audio();
}

void bim::axmol::app::detail::persistent_systems::listen_to_frame_event()
{
  m_frame_event_listener =
      ax::Director::getInstance()
          ->getEventDispatcher()
          ->addCustomEventListener(ax::Director::EVENT_AFTER_DRAW,
                                   [this](ax::EventCustom*)
                                   {
                                     m_application.tick();
                                   });
}

void bim::axmol::app::detail::persistent_systems::remove_frame_event_listener()
{
  ax::Director::getInstance()->getEventDispatcher()->removeEventListener(
      m_frame_event_listener);
}

void bim::axmol::app::detail::persistent_systems::start_root_scene()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: root scene.");

  assert(m_root_scene == nullptr);

  m_root_scene = bim::axmol::app::root_scene::create();

  m_clean_up_connection = m_root_scene->connect_to_clean_up(
      [this]() -> void
      {
        m_clean_up_connection.disconnect();
        m_application.clean_up();
      });

  ax::Director& director = *ax::Director::getInstance();

  director.runWithScene(m_root_scene.get());
  director.setProjection(ax::Director::Projection::_2D);
}

void bim::axmol::app::detail::persistent_systems::stop_root_scene()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: root scene.");

  m_root_scene = nullptr;
}

bim::axmol::app::detail::session_systems::session_systems(application& app)
  : m_application(app)
{
  start_styles();
  start_main_scene();
  start_inputs();
}

bim::axmol::app::detail::session_systems::~session_systems()
{
  stop_inputs();
  stop_main_scene();
  stop_styles();

  ax::SpriteFrameCache::getInstance()->removeSpriteFrames();
  ax::backend::ProgramManager::getInstance()->unloadAllPrograms();
}

const iscool::style::declaration&
bim::axmol::app::detail::session_systems::root_style() const
{
  assert(m_style);
  return *m_style;
}

void bim::axmol::app::detail::session_systems::start_styles()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: styles.");

  iscool::style::initialize({ "style/" });

  m_style.reset(new iscool::style::declaration(
      iscool::style::loader::load("launch/application")));
}

void bim::axmol::app::detail::session_systems::stop_styles()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stop: styles.");

  m_style.reset();

  iscool::style::finalize();
}

void bim::axmol::app::detail::session_systems::start_main_scene()
{
  main_scene* const scene =
      new main_scene(m_application.m_persistent_systems->root_scene(),
                     m_application.m_context.get_widget_context(),
                     *m_style->get_declaration("main-scene"));
  m_application.m_context.set_main_scene(scene);
  m_application.m_input_root.push_back(scene->input_node());
}

void bim::axmol::app::detail::session_systems::stop_main_scene()
{
  m_application.m_input_root.erase(
      m_application.m_context.get_main_scene()->input_node());
  delete m_application.m_context.get_main_scene();
  m_application.m_context.reset_main_scene();
}

void bim::axmol::app::detail::session_systems::start_inputs()
{
  m_input_flow.reset(new bim::axmol::input::flow(
      m_application.m_persistent_systems->root_scene(),
      m_application.m_input_root));
}

void bim::axmol::app::detail::session_systems::stop_inputs()
{
  m_input_flow.reset();
}

bim::axmol::app::application::application()
  : application({}, ax::Size(1, 1), 1, false, nullptr)
{}

bim::axmol::app::application::application(
    std::vector<std::string> asset_directories, const ax::Size& screen_size,
    float screen_scale, bool enable_debug, script_info* script)
  : m_asset_directories(std::move(asset_directories))
  , m_style_cache(m_colors)
  , m_screen_capture_key_observer(ax::EventKeyboard::KeyCode::KEY_C)
  , m_reset_key_observer(ax::EventKeyboard::KeyCode::KEY_R)
  , m_input_root(m_reset_key_observer)
  , m_screen_config{ screen_size, screen_scale }
  , m_script_info(script)
{
  ax::FontFreeType::setStreamParsingEnabled(false);
  ax::Image::setPNGPremultipliedAlphaEnabled(false);

  bim::axmol::action::register_actions(m_action_factory);
  bim::axmol::widget::register_widgets(m_widget_factory);
  bim::axmol::app::register_widgets(m_widget_factory);

  m_reset_key_observer->connect_to_released(
      [this]()
      {
        reset();
      });

  m_input_root.push_back(m_screen_capture_key_observer);
  m_screen_capture_key_observer->connect_to_released(
      [this]()
      {
        capture_screen();
      });

  m_context.set_enable_debug(enable_debug);
}

bim::axmol::app::application::~application() = default;

bool bim::axmol::app::application::applicationDidFinishLaunching()
{
  set_up_file_utils();

  m_persistent_systems.reset(new detail::persistent_systems(*this));

  m_context.get_analytics()->event(
      "launched",
      { { "language",
          iscool::to_string(iscool::system::get_language_name()) } });

  set_up_colour_chart();

  m_context.set_widget_context(bim::axmol::widget::context{
      m_colors, m_style_cache, m_widget_factory, m_action_factory,
      bim::axmol::display::device_scale(1080, 2220) });

  m_session_systems.reset(new detail::session_systems(*this));

  if (m_script_info)
    start_script();

  m_launch_connection = iscool::schedule::delayed_call(
      [this]()
      {
        complete_launch();
      });

  return true;
}

void bim::axmol::app::application::applicationDidEnterBackground()
{
  flush_local_preferences();

  m_context.get_audio()->pause();

  ax::Director::getInstance()->stopAnimation();
}

void bim::axmol::app::application::applicationWillEnterForeground()
{
  ax::Director::getInstance()->startAnimation();

  m_context.get_audio()->resume();
}

void bim::axmol::app::application::complete_launch()
{
  set_up_local_preferences();
  apply_local_preferences();

  launch_game();
}

void bim::axmol::app::application::clean_up()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Quit requested.");

  stop_game();

  m_script_director.reset();

  tear_down_local_preferences();

  m_session_systems.reset();
  m_persistent_systems.reset();
}

void bim::axmol::app::application::reset()
{
  stop_game();

  tear_down_local_preferences();

  m_session_systems.reset();

  // Don't call the overridden cleanup function as it would trigger the
  // clean_up signal and end the game.
  m_persistent_systems->root_scene().ax::Scene::cleanup();

  ax::Director::getInstance()->getTextureCache()->unbindAllImageAsync();
  ax::Director::getInstance()->getTextureCache()->removeAllTextures();

  m_session_systems.reset(new detail::session_systems(*this));

  m_launch_connection = iscool::schedule::delayed_call(
      [this]()
      {
        complete_launch();
      });
}

void bim::axmol::app::application::set_up_file_utils()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: file system.");

  ax::FileUtils& file_utils = *ax::FileUtils::getInstance();

  file_utils.setPopupNotify(false);
  file_utils.setSearchPaths(std::move(m_asset_directories));
}

void bim::axmol::app::application::set_up_colour_chart()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Start: colour chart.");

  const char* const color_file = "colors.json";

  if (!iscool::files::file_exists(color_file))
    {
      ic_log(iscool::log::nature::info(), g_log_context,
             "Could not find color file '{}'.", color_file);
      return;
    }

  const Json::Value colors(iscool::json::from_file(color_file));
  assert(colors.isObject());

  for (Json::Value::const_iterator it = colors.begin(), end = colors.end();
       it != end; ++it)
    m_colors.add_alias(iscool::json::cast<std::string>(it.key()),
                       iscool::json::cast<std::string>(*it));
}

void bim::axmol::app::application::set_up_local_preferences()
{
  ic_log(iscool::log::nature::info(), g_log_context,
         "Start: local preferences.");

  assert(!m_context.is_local_preferences_set());

  constexpr std::chrono::minutes flush_interval(5);
  const std::string path(iscool::files::get_writable_path()
                         + "preferences.json");
  const std::string backup_extension(".bak.{}");
  constexpr int backup_count = 1;

  iscool::preferences::local_preferences* const preferences =
      new iscool::preferences::local_preferences(
          flush_interval, path, backup_extension, backup_count);

  m_context.set_local_preferences(preferences);
}

void bim::axmol::app::application::tear_down_local_preferences()
{
  ic_log(iscool::log::nature::info(), g_log_context,
         "Stop: local preferences.");

  flush_local_preferences();
  delete m_context.get_local_preferences();

  m_context.reset_local_preferences();
}

void bim::axmol::app::application::flush_local_preferences()
{
  if (m_context.is_local_preferences_set())
    m_context.get_local_preferences()->flush();
}

void bim::axmol::app::application::apply_local_preferences()
{
  iscool::preferences::local_preferences& preferences(
      *m_context.get_local_preferences());

  m_context.get_haptic_feedback()->set_enabled(
      bim::app::haptic_feedback_enabled(preferences));

  iscool::audio::mixer& audio = *m_context.get_audio();

  if (!bim::app::music_enabled(preferences))
    audio.set_music_muted(true);

  if (!bim::app::effects_enabled(preferences))
    audio.set_effects_muted(true);
}

void bim::axmol::app::application::launch_game()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Launching the game.");

  m_main_task.reset(new main_task(
      m_context,
      *m_session_systems->root_style().get_declaration("main-task")));
  m_main_task->connect_to_reset(
      [this]() -> void
      {
        ic_log(iscool::log::nature::info(), g_log_context,
               "Resetting the game.");
        reset();
      });

  m_main_task->start();
}

void bim::axmol::app::application::stop_game()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stopping game.");

  flush_local_preferences();
  m_main_task = nullptr;
}

void bim::axmol::app::application::end()
{
  ic_log(iscool::log::nature::info(), g_log_context, "Stopping the game.");

  clean_up();

  ax::Director::getInstance()->end();
}

void bim::axmol::app::application::start_script()
{
#if !__ANDROID__
  assert(m_script_info);

  m_script_director.reset(new script_director(
      application_event_listener(*m_context.get_event_dispatcher()),
      m_script_info->file_path, m_script_info->number_screenshots,
      m_script_info->timeout));
  m_script_director->connect_to_done(
      [this](script_director::result r)
      {
        m_script_info->passed = (r == script_director::result::ok);
        end();
      });
#endif
}

void bim::axmol::app::application::tick()
{
  m_context.get_audio()->tick();
}

void bim::axmol::app::application::capture_screen() const
{
  ax::utils::captureScreen(
      [](ax::RefPtr<ax::Image> image) -> void
      {
        std::ostringstream oss;
        const std::time_t t = std::time(nullptr);
        oss << std::put_time(std::gmtime(&t), "%Y-%m-%d_%H-%M-%S") << ".png";

        image->saveToFile(oss.str());
      });
}
