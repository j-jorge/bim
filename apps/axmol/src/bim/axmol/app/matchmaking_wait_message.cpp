// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/matchmaking_wait_message.hpp>

#include <iscool/files/read_file.hpp>
#include <iscool/i18n/gettext.hpp>
#include <iscool/monitoring/implement_state_monitor.hpp>
#include <iscool/random/rand.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/system/language_code.hpp>

ic_implement_state_monitor(bim::axmol::app::matchmaking_wait_message,
                           m_monitor, stopped,
                           ((stopped)((started)))         //
                           ((started)((paused)(stopped))) //
                           ((paused)((started)(stopped))));

IMPLEMENT_SIGNAL(bim::axmol::app::matchmaking_wait_message, updated, m_updated)

bim::axmol::app::matchmaking_wait_message::matchmaking_wait_message()
  : m_current_script(0)
  , m_current_line(0)
{
  m_scripts.push_back(vector_of_strings(
      { ic_gettext("Please wait."), ic_gettext("Please wait.."),
        ic_gettext("Please wait..."), ic_gettext("Please wait..") }));

  load_messages(iscool::system::get_language_code());

  if (m_scripts.size() == 1)
    load_messages("en");
}

bim::axmol::app::matchmaking_wait_message::~matchmaking_wait_message() =
    default;

void bim::axmol::app::matchmaking_wait_message::start()
{
  if (m_monitor->is_started_state())
    return;

  if (m_monitor->is_stopped_state())
    {
      iscool::random::rand::get_default().random_shuffle(m_scripts.begin() + 1,
                                                         m_scripts.end());
      start_default_script();
      m_next_script = 1;
    }

  m_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        schedule_tick();
        dispatch_updated();
      });

  m_monitor->set_started_state();
}

void bim::axmol::app::matchmaking_wait_message::pause()
{
  if (!m_monitor->is_started_state())
    return;

  m_connection.disconnect();
  m_monitor->set_paused_state();
}

void bim::axmol::app::matchmaking_wait_message::stop()
{
  if (m_monitor->is_stopped_state())
    return;

  m_connection.disconnect();
  m_monitor->set_stopped_state();
}

void bim::axmol::app::matchmaking_wait_message::load_messages(
    std::string_view language_code)
{
  std::string base_name = "matchmaking/";
  base_name += language_code;
  base_name += '-';

  const std::string extension = ".txt";
  std::string file_name;
  file_name.reserve(base_name.size() + 3 + extension.size() + 1);

  std::string line;

  for (int i = 0; i != 255; ++i)
    {
      file_name = base_name;
      file_name += std::to_string(i);
      file_name += extension;

      const std::unique_ptr<std::istream> f =
          iscool::files::read_file(file_name);

      if (!f)
        break;

      vector_of_strings script;
      script.reserve(8);
      std::string text;

      // Read the script from the file. Messages are separated by empty lines.
      while (std::getline(*f, line))
        if (line.empty())
          {
            if (!text.empty())
              {
                script.push_back(text);
                text.clear();
              }
          }
        else
          {
            if (!text.empty())
              text += '\n';

            text += line;
          }

      if (!text.empty())
        script.push_back(std::move(text));

      if (!script.empty())
        m_scripts.push_back(std::move(script));
    }
}

void bim::axmol::app::matchmaking_wait_message::start_default_script()
{
  m_current_script = 0;
  m_current_line = 0;
  m_remaining_loops_on_default_script = 3;
}

void bim::axmol::app::matchmaking_wait_message::next_script()
{
  assert(m_next_script != 0);
  m_current_line = 0;

  if (m_current_script == 0)
    {
      // On the default script there are many iterations.
      assert(m_remaining_loops_on_default_script > 0);
      --m_remaining_loops_on_default_script;

      if (m_remaining_loops_on_default_script == 0)
        {
          // The default script is over. If we have played all scripts, we
          // schedule the whole loop again (repeating the default
          // script). Otherwise we just switch to the next script.

          m_current_script = m_next_script;

          if (m_current_script == m_scripts.size())
            {
              start_default_script();
              m_next_script = 1;
            }
        }
      // else we continue with the next loop of the default script.
    }
  else
    {
      // The non-default scripts are played once and switch to the default
      // script before switching to the next in the list.
      start_default_script();
      m_next_script = m_next_script + 1;
    }
}

void bim::axmol::app::matchmaking_wait_message::schedule_tick()
{
  std::chrono::milliseconds delay =
      m_scripts[m_current_script][m_current_line].size()
      * std::chrono::milliseconds(80);

  constexpr std::chrono::seconds min_delay(1);

  if (delay < min_delay)
    delay = min_delay;

  m_connection = iscool::schedule::delayed_call(
      [this]() -> void
      {
        tick();
      },
      delay);
}

void bim::axmol::app::matchmaking_wait_message::tick()
{
  ++m_current_line;

  if (m_current_line == m_scripts[m_current_script].size())
    next_script();

  schedule_tick();
  dispatch_updated();
}

void bim::axmol::app::matchmaking_wait_message::dispatch_updated()
{
  assert(m_current_script < m_scripts.size());
  assert(m_current_line < m_scripts[m_current_script].size());

  m_updated(m_scripts[m_current_script][m_current_line]);
}
