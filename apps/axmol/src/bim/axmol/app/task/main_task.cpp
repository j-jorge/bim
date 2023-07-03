#include <bim/axmol/app/task/main_task.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/signals/implement_signal.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, end, m_end);

bim::axmol::app::main_task::main_task(context context)
  : m_context(context)
{}

void bim::axmol::app::main_task::start()
{
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);
}
