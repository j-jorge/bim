// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/settings_popup.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>
#include <bim/axmol/app/popup/language_popup.hpp>
#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/toggle.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <bim/app/analytics/button_clicked.hpp>
#include <bim/app/analytics_service.hpp>
#include <bim/app/preference/audio.hpp>
#include <bim/app/preference/controls.hpp>
#include <bim/app/preference/haptic.hpp>
#include <bim/app/preference/user_language.hpp>

#include <bim/version.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/social/service.hpp>
#include <iscool/system/open_url.hpp>
#include <iscool/system/send_mail.hpp>

#include <axmol/2d/Label.h>
#include <axmol/base/EventKeyboard.h>

#define x_widget_scope bim::axmol::app::settings_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button,                                        \
           close_button) x_widget(bim::axmol::widget::button, bluesky_button) \
      x_widget(bim::axmol::widget::button, github_button) x_widget(           \
          bim::axmol::widget::button,                                         \
          mail_button) x_widget(bim::axmol::widget::button, share_button)     \
          x_widget(bim::axmol::widget::toggle, music)                         \
              x_widget(bim::axmol::widget::toggle, sound_effects)             \
                  x_widget(bim::axmol::widget::toggle, vibrations)            \
                      x_widget(bim::axmol::widget::toggle, d_pad_position)    \
                          x_widget(bim::axmol::widget::toggle, d_pad_kind)    \
                              x_widget(bim::axmol::widget::button,            \
                                       language_button)                       \
                                  x_widget(ax::Label, version)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/audio/mixer.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/system/haptic_feedback.hpp>

#include <fmt/format.h>

IMPLEMENT_SIGNAL(bim::axmol::app::settings_popup, reset, m_reset);

bim::axmol::app::settings_popup::settings_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_style_pad_on_the_left(
        *style.get_declaration("display.d-pad-on-the-left"))
  , m_style_pad_on_the_right(
        *style.get_declaration("display.d-pad-on-the-right"))
  , m_style_directions_stick(*style.get_declaration("display.d-pad-stick"))
  , m_style_directions_pad(*style.get_declaration("display.d-pad-pad"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
  , m_language_popup(
        new language_popup(context, *style.get_declaration("language-popup")))
{
  ax::Label* const language_label = (ax::Label*)find_child_by_path(
      *m_controls->language_button,
      *style.get_string("language-button-label-path"));
  language_label->setString(iscool::to_human_string(
      bim::app::user_language(*context.get_local_preferences())));

  m_inputs.push_back(m_escape);
  m_inputs.push_back(m_controls->close_button->input_node());
  m_inputs.push_back(m_controls->bluesky_button->input_node());
  m_inputs.push_back(m_controls->github_button->input_node());
  m_inputs.push_back(m_controls->mail_button->input_node());
  m_inputs.push_back(m_controls->share_button->input_node());
  m_inputs.push_back(m_controls->music->input_node());
  m_inputs.push_back(m_controls->sound_effects->input_node());
  m_inputs.push_back(m_controls->vibrations->input_node());
  m_inputs.push_back(m_controls->d_pad_position->input_node());
  m_inputs.push_back(m_controls->d_pad_kind->input_node());
  m_inputs.push_back(m_controls->language_button->input_node());

  const std::string version =
      fmt::format(fmt::runtime(ic_gettext("Version {}")), bim::version);

  m_controls->version->setString(version);

  m_controls->close_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
      });

  m_escape->connect_to_released(
      [this]()
      {
        m_popup->hide();
      });

  m_controls->language_button->connect_to_clicked(
      [this]()
      {
        show_language_selection();
      });
  m_language_popup->connect_to_reset(
      [this]()
      {
        m_reset();
      });

  m_controls->bluesky_button->connect_to_clicked(
      [this]()
      {
        open_bluesky();
      });

  m_controls->github_button->connect_to_clicked(
      [this]()
      {
        open_github();
      });

  m_controls->mail_button->connect_to_clicked(
      [this]()
      {
        open_mail();
      });

  m_controls->share_button->connect_to_clicked(
      [this]()
      {
        open_share();
      });

  m_controls->music->connect_to_clicked(
      [this]()
      {
        set_music_preference();
      });

  m_controls->sound_effects->connect_to_clicked(
      [this]()
      {
        set_sound_effects_preference();
      });

  m_controls->vibrations->connect_to_clicked(
      [this]()
      {
        set_vibrations_preference();
      });

  m_controls->d_pad_position->connect_to_clicked(
      [this]()
      {
        set_d_pad_position_preference();
      });

  m_controls->d_pad_kind->connect_to_clicked(
      [this]()
      {
        set_d_pad_kind_preference();
      });
}

bim::axmol::app::settings_popup::~settings_popup() = default;

void bim::axmol::app::settings_popup::show()
{
  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  m_controls->music->set_state(bim::app::music_enabled(preferences));
  m_controls->sound_effects->set_state(bim::app::effects_enabled(preferences));
  m_controls->vibrations->set_state(
      bim::app::haptic_feedback_enabled(preferences));

  set_direction_pad_display(bim::app::direction_pad_on_the_left(preferences));
  set_stick_or_pad_display(bim::app::direction_pad_kind_is_stick(preferences));

  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());
}

void bim::axmol::app::settings_popup::set_direction_pad_display(
    bool pad_on_the_left)
{
  m_controls->d_pad_position->set_state(pad_on_the_left);

  bim::axmol::widget::apply_display(
      m_context.get_widget_context().style_cache, m_controls->all_nodes,
      pad_on_the_left ? m_style_pad_on_the_left : m_style_pad_on_the_right);
}

void bim::axmol::app::settings_popup::set_stick_or_pad_display(bool use_stick)
{
  m_controls->d_pad_kind->set_state(use_stick);

  bim::axmol::widget::apply_display(
      m_context.get_widget_context().style_cache, m_controls->all_nodes,
      use_stick ? m_style_directions_stick : m_style_directions_pad);
}

void bim::axmol::app::settings_popup::set_music_preference()
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bool v = !bim::app::music_enabled(preferences);
  bim::app::music_enabled(preferences, v);
  m_controls->music->set_state(v);

  m_context.get_analytics()->event("preference",
                                   { { "music", v ? "true" : "false" } });

  m_context.get_audio()->set_music_muted(!v);
}

void bim::axmol::app::settings_popup::set_sound_effects_preference()
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bool v = !bim::app::effects_enabled(preferences);
  bim::app::effects_enabled(preferences, v);
  m_controls->sound_effects->set_state(v);

  m_context.get_analytics()->event("preference",
                                   { { "fx", v ? "true" : "false" } });

  m_context.get_audio()->set_effects_muted(!v);
}

void bim::axmol::app::settings_popup::set_vibrations_preference()
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bool v = !bim::app::haptic_feedback_enabled(preferences);
  bim::app::haptic_feedback_enabled(preferences, v);
  m_controls->vibrations->set_state(v);

  m_context.get_analytics()->event("preference",
                                   { { "vibrations", v ? "true" : "false" } });

  m_context.get_haptic_feedback()->set_enabled(v);
}

void bim::axmol::app::settings_popup::set_d_pad_position_preference()
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bool v = !bim::app::direction_pad_on_the_left(preferences);
  bim::app::direction_pad_on_the_left(preferences, v);

  m_context.get_analytics()->event(
      "preference", { { "d-pad-position", v ? "left" : "right" } });

  set_direction_pad_display(v);
}

void bim::axmol::app::settings_popup::set_d_pad_kind_preference()
{
  iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  const bool use_stick = !bim::app::direction_pad_kind_is_stick(preferences);
  bim::app::direction_pad_kind_is_stick(preferences, use_stick);

  m_context.get_analytics()->event(
      "preference", { { "d-pad-kind", use_stick ? "stick" : "cross" } });

  set_stick_or_pad_display(use_stick);
}

void bim::axmol::app::settings_popup::show_language_selection()
{
  bim::app::button_clicked(*m_context.get_analytics(), "language", "settings");
  m_language_popup->show();
}

void bim::axmol::app::settings_popup::open_bluesky()
{
  bim::app::button_clicked(*m_context.get_analytics(), "bluesky", "settings");
  iscool::system::open_url("https://bsky.app/profile/j-jorge.bsky.social");
  m_context.get_event_dispatcher()->dispatch("bluesky");
}

void bim::axmol::app::settings_popup::open_github()
{
  bim::app::button_clicked(*m_context.get_analytics(), "github", "settings");
  iscool::system::open_url("https://github.com/j-jorge/bim/");
  m_context.get_event_dispatcher()->dispatch("github");
}

void bim::axmol::app::settings_popup::open_mail()
{
  bim::app::button_clicked(*m_context.get_analytics(), "mail", "settings");
  iscool::system::send_mail("bim-game@gmx.com",
                            ic_gettext("Feedback about Bim!"), "");
  m_context.get_event_dispatcher()->dispatch("mail");
}

void bim::axmol::app::settings_popup::open_share()
{
  bim::app::button_clicked(*m_context.get_analytics(), "share", "settings");
  m_context.get_social()->share_message(
      ic_gettext("Come play a game of Bim! "
                 "https://play.google.com/store/apps/details?id=bim.app"));
  m_context.get_event_dispatcher()->dispatch("share");
}
