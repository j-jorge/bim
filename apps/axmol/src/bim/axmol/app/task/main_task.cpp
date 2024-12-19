// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/task/main_task.hpp>

#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/screen_wheel.hpp>

#include <bim/net/message/authentication_error_code.hpp>

#include <iscool/audio/loop_mode.hpp>
#include <iscool/audio/mixer.hpp>
#include <iscool/files/file_exists.hpp>
#include <iscool/files/read_file.hpp>
#include <iscool/i18n/gettext.hpp>
#include <iscool/i18n/load_translations.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>
#include <iscool/system/language_code.hpp>

#include <fmt/format.h>

IMPLEMENT_SIGNAL(bim::axmol::app::main_task, end, m_end);

bim::axmol::app::main_task::main_task(context context)
  : m_context(context)
  , m_style(iscool::style::loader::load("main-task"))
{
  m_context.set_session_handler(&m_session_handler);
}

bim::axmol::app::main_task::~main_task() = default;

void bim::axmol::app::main_task::start()
{
  m_context.get_audio()->play_music("menu", iscool::audio::loop_mode::forever);

  read_translations();

  m_message_popup.reset(
      new message_popup(m_context, *m_style.get_declaration("message-popup")));

  connect_to_game_server();

  m_screen_wheel.reset(
      new screen_wheel(m_context, *m_style.get_declaration("screen-wheel")));
}

void bim::axmol::app::main_task::read_translations()
{
  const std::string language_code = iscool::system::get_language_code();
  std::string translations_file = "i18n/" + language_code + ".mo";

  if (!iscool::files::file_exists(translations_file))
    translations_file = "i18n/en.mo";

  const std::unique_ptr<std::istream> mo_file =
      iscool::files::read_file(translations_file);

  if (!iscool::i18n::load_translations(language_code, *mo_file))
    ic_log(iscool::log::nature::warning(), "main_task",
           "Could not read translations from {}.\n", translations_file);
}

void bim::axmol::app::main_task::connect_to_game_server()
{
  m_session_config_error_connection =
      m_session_handler.connect_to_config_error(
          [this]()
          {
            m_message_popup->show(
                ic_gettext("Could not get the config from the server."));
          });

  m_session_authentication_error_connection =
      m_session_handler.connect_to_authentication_error(
          [this](bim::net::authentication_error_code error_code)
          {
            m_message_popup->show(fmt::format(
                fmt::runtime(ic_gettext("Failed to authenticate with the "
                                        "game server. Error code {}.")),
                std::underlying_type_t<bim::net::authentication_error_code>(
                    error_code)));
          });

  m_session_handler.connect();
}
