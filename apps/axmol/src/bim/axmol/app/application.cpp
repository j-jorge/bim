#include <bim/axmol/app/application.hpp>

#include <bim/axmol/app/preference/audio.hpp>
#include <bim/axmol/app/preference/haptic.hpp>
#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/main_scene.hpp>
#include <bim/axmol/app/root_scene.hpp>
#include <bim/axmol/app/scene_lock.hpp>

#include <bim/axmol/audio/mixer.hpp>
#include <bim/axmol/input/flow.hpp>
#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/node.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>
#include <bim/axmol/widget/register_widgets.hpp>

#include <iscool/audio/default_mixer.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/file_exists.hpp>
#include <iscool/files/get_writable_path.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/log/add_file_sink.hpp>
#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/info.hpp>
#include <iscool/log/setup.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/style/setup.hpp>
#include <iscool/system/device_date.hpp>
#include <iscool/system/haptic_feedback.hpp>

#include <axmol/2d/Scene.h>
#include <axmol/2d/SpriteFrameCache.h> // TODO: remove
#include <axmol/base/Director.h>
#include <axmol/base/EventDispatcher.h>
#include <axmol/platform/FileUtils.h>

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

  void start_haptic_feedback();
  void stop_haptic_feedback();

  void start_audio();
  void stop_audio();

  void start_root_scene();
  void stop_root_scene();

private:
  application& m_application;
  bim::axmol::audio::mixer m_audio;
  bim::axmol::ref_ptr<bim::axmol::app::root_scene> m_root_scene;
  iscool::signals::scoped_connection m_clean_up_connection;
};

struct bim::axmol::app::detail::session_systems
{
public:
  explicit session_systems(application& app);
  session_systems(const session_systems&) = delete;
  ~session_systems();

private:
  void start_styles();
  void stop_styles();

  void start_main_scene();
  void stop_main_scene();

  void start_inputs();
  void stop_inputs();

  void start_lock();
  void stop_lock();

private:
  application& m_application;

  std::unique_ptr<bim::axmol::input::flow> m_input_flow;
};

bim::axmol::app::detail::persistent_systems::persistent_systems(
    application& app)
  : m_application(app)
{
  start_log_system();
  start_display();
  start_haptic_feedback();
  start_audio();
  start_root_scene();
}

bim::axmol::app::detail::persistent_systems::~persistent_systems()
{
  stop_root_scene();
  stop_audio();
  stop_haptic_feedback();
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
  iscool::log::initialize();

  const std::string log_file_path(iscool::files::get_writable_path()
                                  + "log.txt");

  iscool::log::add_file_sink(log_file_path);

  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Log system initialized");

  std::tm now(iscool::system::device_date());
  char now_str[32];

  if (std::strftime(now_str, sizeof(now_str), "%FT%T%z", &now) == 0)
    ic_causeless_log(iscool::log::nature::info(), "StartUp",
                     "Failed to get the date.");
  else
    ic_causeless_log(iscool::log::nature::info(), g_log_context, "%1%",
                     static_cast<const char*>(now_str));
}

void bim::axmol::app::detail::persistent_systems::stop_log_system()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stop: log system.");

  iscool::log::finalize();
}

void bim::axmol::app::detail::persistent_systems::start_display()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: display.");
  assert(m_application.m_main_view == nullptr);
  m_application.m_main_view.reset(new bim::axmol::display::main_view(
      "Bim!", m_application.m_screen_config.size,
      m_application.m_screen_config.scale));
}

void bim::axmol::app::detail::persistent_systems::stop_display()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stop: display.");

  m_application.m_main_view.reset();
}

void bim::axmol::app::detail::persistent_systems::start_haptic_feedback()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: haptic feedback.");

  assert(!m_application.m_context.is_haptic_feedback_set());

  iscool::system::haptic_feedback* const feedback(
      new iscool::system::haptic_feedback());

  m_application.m_context.set_haptic_feedback(feedback);
}

void bim::axmol::app::detail::persistent_systems::stop_haptic_feedback()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stop: haptic feedback.");

  delete m_application.m_context.get_haptic_feedback();
  m_application.m_context.reset_haptic_feedback();
}

void bim::axmol::app::detail::persistent_systems::start_audio()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: audio.");

  iscool::audio::mixer* const mixer(new iscool::audio::mixer(
      iscool::resources::resolver("audio/", ".ogg"), 5, m_audio));

  iscool::audio::set_default_mixer(*mixer);
  m_application.m_context.set_audio(mixer);
}

void bim::axmol::app::detail::persistent_systems::stop_audio()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context, "Stop: audio.");

  iscool::audio::clear_default_mixer();
  delete m_application.m_context.get_audio();
  m_application.m_context.reset_audio();
}

void bim::axmol::app::detail::persistent_systems::start_root_scene()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: root scene.");

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
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stop: root scene.");

  m_root_scene = nullptr;
}

bim::axmol::app::detail::session_systems::session_systems(application& app)
  : m_application(app)
{
  start_styles();
  start_main_scene();
  start_inputs();
  start_lock();
}

bim::axmol::app::detail::session_systems::~session_systems()
{
  stop_lock();
  stop_inputs();
  stop_main_scene();
  stop_styles();
}

void bim::axmol::app::detail::session_systems::start_styles()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: styles.");

  iscool::style::initialize({ "style/" });
}

void bim::axmol::app::detail::session_systems::stop_styles()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stop: styles.");

  iscool::style::finalize();
}

void bim::axmol::app::detail::session_systems::start_main_scene()
{
  main_scene* const scene(
      new main_scene(m_application.m_persistent_systems->root_scene(),
                     m_application.m_context.get_widget_context(),
                     iscool::style::loader::load("launch/main-scene")));
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

void bim::axmol::app::detail::session_systems::start_lock()
{
  assert(!m_application.m_context.is_scene_lock_set());
  assert(m_application.m_context.get_main_scene() != nullptr);

  m_application.m_context.set_scene_lock(
      new scene_lock(*m_application.m_context.get_main_scene(),
                     iscool::style::loader::load("launch/scene-lock")));
}

void bim::axmol::app::detail::session_systems::stop_lock()
{
  delete m_application.m_context.get_scene_lock();
  m_application.m_context.reset_scene_lock();
}

bim::axmol::app::application::application()
  : application({}, ax::Size(1, 1), 1)
{}

bim::axmol::app::application::application(
    std::vector<std::string> asset_directories, const ax::Size& screen_size,
    float screen_scale)
  : m_asset_directories(std::move(asset_directories))
  , m_style_cache(m_colors)
  , m_reset_key_observer(ax::EventKeyboard::KeyCode::KEY_R)
  , m_input_root(m_reset_key_observer)
  , m_screen_config(screen_size, screen_scale)
{
  bim::axmol::widget::register_widgets(m_widget_factory);

  m_reset_key_observer->connect_to_released(
      [this]()
      {
        reset();
      });
}

bim::axmol::app::application::~application() = default;

bool bim::axmol::app::application::applicationDidFinishLaunching()
{
  set_up_file_utils();

  m_persistent_systems.reset(new detail::persistent_systems(*this));

  set_up_colour_chart();

  m_context.set_widget_context(
      bim::axmol::widget::context(m_colors, m_style_cache, m_widget_factory));

  // TODO: in a loader.
  ax::SpriteFrameCache::getInstance()->addSpriteFramesWithFile(
      "sprite-sheet.plist");
  m_session_systems.reset(new detail::session_systems(*this));

  m_context.get_scene_lock()->instant_lock();

  m_launch_connection = iscool::schedule::delayed_call(
      std::bind(&application::complete_launch, this));

  return true;
}

void bim::axmol::app::application::applicationDidEnterBackground()
{
  flush_local_preferences();
  ax::Director::getInstance()->stopAnimation();
}

void bim::axmol::app::application::applicationWillEnterForeground()
{
  ax::Director::getInstance()->startAnimation();
}

void bim::axmol::app::application::complete_launch()
{
  set_up_local_preferences();
  apply_local_preferences();

  listen_to_frame_event();

  launch_game();
}

void bim::axmol::app::application::clean_up()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Quit requested.");

  stop_game();

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
  m_session_systems.reset(new detail::session_systems(*this));

  m_launch_connection = iscool::schedule::delayed_call(
      std::bind(&application::complete_launch, this));
}

void bim::axmol::app::application::set_up_file_utils()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: file system.");

  ax::FileUtils& file_utils = *ax::FileUtils::getInstance();

  file_utils.setPopupNotify(false);
  file_utils.setSearchPaths(std::move(m_asset_directories));
}

void bim::axmol::app::application::set_up_colour_chart()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: colour chart.");

  const char* const color_file = "colors.json";

  if (!iscool::files::file_exists(color_file))
    {
      ic_causeless_log(iscool::log::nature::info(), g_log_context,
                       "Could not find color file '%s'.", color_file);
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
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Start: local preferences.");

  assert(!m_context.is_local_preferences_set());

  constexpr std::chrono::minutes flush_interval(5);
  const std::string path(iscool::files::get_writable_path()
                         + "preferences.json");
  const std::string backup_extension(".bak.%1%");
  constexpr int backup_count = 1;

  iscool::preferences::local_preferences* const preferences =
      new iscool::preferences::local_preferences(
          flush_interval, path, backup_extension, backup_count);

  m_context.set_local_preferences(preferences);
}

void bim::axmol::app::application::tear_down_local_preferences()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
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
      haptic_feedback_enabled(preferences));

  iscool::audio::mixer& audio = *m_context.get_audio();

  if (!music_enabled(preferences))
    audio.set_music_muted(true);

  if (!effects_enabled(preferences))
    audio.set_effects_muted(true);
}

void bim::axmol::app::application::launch_game()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Launching the game.");

  m_main_task.reset(new main_task(m_context));
  m_main_task->connect_to_end(
      [this]() -> void
      {
        ic_causeless_log(iscool::log::nature::info(), g_log_context,
                         "Stopping the game.");
        stop_game();
        m_session_systems = nullptr;
        m_persistent_systems = nullptr;
        ax::Director::getInstance()->end();
      });

  m_main_task->start();
}

void bim::axmol::app::application::stop_game()
{
  ic_causeless_log(iscool::log::nature::info(), g_log_context,
                   "Stopping game.");

  flush_local_preferences();
  m_main_task = nullptr;
}

void bim::axmol::app::application::listen_to_frame_event()
{
  ax::Director& director(*ax::Director::getInstance());

  director.getEventDispatcher()->addCustomEventListener(
      ax::Director::EVENT_AFTER_DRAW,
      std::bind(&bim::axmol::app::application::tick, this));
}

void bim::axmol::app::application::tick()
{
  m_context.get_audio()->tick();
}
