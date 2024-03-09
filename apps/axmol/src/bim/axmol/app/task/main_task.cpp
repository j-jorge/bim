#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/screen_wheel.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/file_exists.hpp>
#include <iscool/files/read_file.hpp>
#include <iscool/i18n/load_translations.hpp>
#include <iscool/log/causeless_log.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/system/language_code.hpp>

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, end, m_end);

bim::axmol::app::main_task::main_task(context context)
  : m_context(context)
{
  m_context.set_session_handler(&m_session_handler);
}

bim::axmol::app::main_task::~main_task() = default;

void bim::axmol::app::main_task::start()
{
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);

  m_session_handler.connect("localhost:23899");

  const std::string language_code = iscool::system::get_language_code();
  std::string translations_file = "i18n/" + language_code + ".mo";

  if (!iscool::files::file_exists(translations_file))
    translations_file = "i18n/en.mo";

  const std::unique_ptr<std::istream> mo_file =
      iscool::files::read_file(translations_file);

  if (!iscool::i18n::load_translations(language_code, *mo_file))
    ic_causeless_log(iscool::log::nature::warning(), "main_task",
                     "Could not read translations from %s.\n",
                     translations_file);

  // TODO: in order:
  // - parallel
  //   - load resources
  //   - connect to game server
  // - build screen_wheel
  // - unlock screen.
  m_screen_wheel.reset(new screen_wheel(
      m_context, iscool::style::loader::load("app/screen-wheel")));
}
