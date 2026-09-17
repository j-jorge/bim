// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/nickname_editor_popup.hpp>

#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/edit_box.hpp>
#include <bim/axmol/widget/ui/passive_node.hpp>

#include <axmol/2d/Label.h>

#define x_widget_scope bim::axmol::app::nickname_editor_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, close_button)                          \
      x_widget(ax::Label, constraints_label)                                  \
          x_widget(bim::axmol::widget::edit_box, edit_box)                    \
              x_widget(ax::Label, error_label)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <bim/app/business/player_profile.hpp>
#include <bim/app/config.hpp>
#include <bim/app/job/update_nickname_job.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/base/UTF8.h>

IMPLEMENT_SIGNAL(bim::axmol::app::nickname_editor_popup, closed, m_closed);

bim::axmol::app::nickname_editor_popup::nickname_editor_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(*context.get_widget_context(),
               *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
  , m_message_popup(
        new message_popup(context, *style.get_declaration("message-popup")))
  , m_job(new bim::app::update_nickname_job(*m_context.get_analytics(),
                                            *m_context.get_request_headers(),
                                            *m_context.get_player_profile()))
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
{
  m_controls->constraints_label->setString(
      fmt::format(fmt::runtime(ic_gettext(
                      "Your nickname must have between {} and {} symbols.")),
                  m_context.get_config()->nickname_length_min,
                  m_context.get_config()->nickname_length_max));

  m_inputs.push_back(m_controls->edit_box->input_node());
  m_inputs.push_back(m_controls->close_button->input_node());
  m_inputs.push_back(m_escape);

  m_controls->close_button->connect_to_clicked(
      [this]()
        {
          validate();
        });
  m_escape->connect_to_released(
      [this]()
        {
          close();
        });

  m_controls->edit_box->connect_to_changed(
      [this]()
        {
          check_new_nickname();
        });
  m_controls->edit_box->connect_to_cancel(
      [this]()
        {
          reset_display();
        });
  m_message_popup->connect_to_ok(
      [this]()
        {
          start_job();
        });
  m_message_popup->connect_to_nope(
      [this]()
        {
          close();
        });
}

bim::axmol::app::nickname_editor_popup::~nickname_editor_popup() = default;

void bim::axmol::app::nickname_editor_popup::show()
{
  reset_display();
  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());
}

void bim::axmol::app::nickname_editor_popup::reset_display()
{
  m_controls->edit_box->set_text(
      m_context.get_player_profile()->nickname.c_str());
  m_controls->error_label->setString("");

  enable(true);
}

void bim::axmol::app::nickname_editor_popup::enable(bool enabled)
{
  m_controls->close_button->enable(enabled);
  m_controls->edit_box->enable(enabled);
  m_escape->set_enabled(enabled);
}

void bim::axmol::app::nickname_editor_popup::check_new_nickname()
{
  m_clean_nickname.clear();

  const std::string_view nickname = m_controls->edit_box->get_text();

  if (!ax::StringUtils::isLegalUTF8String(nickname.data(), nickname.size()))
    {
      m_controls->error_label->setString(ic_gettext("Invalid nickname."));
      return;
    }

  std::u32string nickname_utf32;
  ax::StringUtils::UTF8ToUTF32(nickname, nickname_utf32);

  const std::u32string::const_iterator start =
      std::find_if(nickname_utf32.cbegin(), nickname_utf32.cend(),
                   [](char32_t c)
                     {
                       return !ax::StringUtils::isUnicodeSpace(c);
                     });
  const std::u32string::const_reverse_iterator rstart =
      std::find_if(nickname_utf32.crbegin(), nickname_utf32.crend(),
                   [](char32_t c)
                     {
                       return !ax::StringUtils::isUnicodeSpace(c);
                     });

  const std::u32string_view nickname_utf32_trimmed(&*start,
                                                   &*rstart - &*start + 1);

  const bim::app::config& config = *m_context.get_config();

  if (nickname_utf32_trimmed.size() < config.nickname_length_min)
    {
      m_controls->error_label->setString(ic_gettext("Too short."));
      return;
    }

  if (nickname_utf32_trimmed.size() > config.nickname_length_max)
    {
      m_controls->error_label->setString(ic_gettext("Too long."));
      return;
    }

  ax::StringUtils::UTF32ToUTF8(nickname_utf32_trimmed, m_clean_nickname);
  m_controls->error_label->setString("");
}

void bim::axmol::app::nickname_editor_popup::validate()
{
  if (m_clean_nickname.empty()
      || (m_clean_nickname == m_context.get_player_profile()->nickname))
    {
      close();
      return;
    }

  m_message_popup->show_yes_no(
      fmt::format(fmt::runtime(ic_gettext(
                      "Change your nickname for \"{}\"? You won't be able to "
                      "change it again before a couple of hours.")),
                  m_clean_nickname));
}

void bim::axmol::app::nickname_editor_popup::start_job()
{
  enable(false);

  m_error_connection = m_job->connect_to_error(
      [this]()
        {
          reset_display();
          m_controls->error_label->setString(ic_gettext("Request error."));
        });
  m_done_connection = m_job->connect_to_done(
      [this]()
        {
          close();
        });

  m_job->start(std::string(m_clean_nickname));
}

void bim::axmol::app::nickname_editor_popup::close()
{
  m_done_connection.disconnect();
  m_error_connection.disconnect();

  m_popup->hide();
  m_closed();
}
