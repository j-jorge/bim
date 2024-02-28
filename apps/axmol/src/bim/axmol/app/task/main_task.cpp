#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/screen_wheel.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, end, m_end);

bim::axmol::app::main_task::main_task(context context)
  : m_context(context)
{}

bim::axmol::app::main_task::~main_task() = default;

void bim::axmol::app::main_task::start()
{
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);

  // TODO: in order:
  // - parallel
  //   - load resources
  //   - connect to game server
  // - build screen_wheel
  // - unlock screen.
  m_screen_wheel.reset(new screen_wheel(
      m_context, iscool::style::loader::load("app/screen-wheel")));
}
