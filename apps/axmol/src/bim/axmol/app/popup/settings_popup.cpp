// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/settings_popup.hpp>

#include <bim/axmol/app/popup/popup.hpp>

#include <bim/axmol/app/preference/audio.hpp>
#include <bim/axmol/app/preference/controls.hpp>
#include <bim/axmol/app/preference/haptic.hpp>

#include <bim/axmol/widget/apply_display.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/toggle.hpp>

#define x_widget_scope bim::axmol::app::settings_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, close_button)                          \
      x_widget(bim::axmol::widget::toggle, music)                             \
          x_widget(bim::axmol::widget::toggle, sound_effects)                 \
              x_widget(bim::axmol::widget::toggle, vibrations)                \
                  x_widget(bim::axmol::widget::toggle, d_pad_position)        \
                      x_widget(bim::axmol::widget::toggle, d_pad_kind)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/audio/mixer.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/system/haptic_feedback.hpp>

bim::axmol::app::settings_popup::settings_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_style_pad_on_the_left(
        *style.get_declaration("display.d-pad-on-the-left"))
  , m_style_pad_on_the_right(
        *style.get_declaration("display.d-pad-on-the-right"))
  , m_style_directions_stick(*style.get_declaration("display.d-pad-stick"))
  , m_style_directions_pad(*style.get_declaration("display.d-pad-pad"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
{
  m_inputs.push_back(m_controls->close_button->input_node());
  m_inputs.push_back(m_controls->music->input_node());
  m_inputs.push_back(m_controls->sound_effects->input_node());
  m_inputs.push_back(m_controls->vibrations->input_node());
  m_inputs.push_back(m_controls->d_pad_position->input_node());
  m_inputs.push_back(m_controls->d_pad_kind->input_node());

  m_controls->close_button->connect_to_clicked(
      [this]()
      {
        m_popup->hide();
      });

  m_controls->music->connect_to_clicked(
      [this]()
      {
        iscool::preferences::local_preferences& preferences =
            *m_context.get_local_preferences();

        const bool v = !music_enabled(preferences);
        music_enabled(preferences, v);
        m_controls->music->set_state(v);

        m_context.get_audio()->set_music_muted(!v);
      });

  m_controls->sound_effects->connect_to_clicked(
      [this]()
      {
        iscool::preferences::local_preferences& preferences =
            *m_context.get_local_preferences();

        const bool v = !effects_enabled(preferences);
        effects_enabled(preferences, v);
        m_controls->sound_effects->set_state(v);

        m_context.get_audio()->set_effects_muted(!v);
      });

  m_controls->vibrations->connect_to_clicked(
      [this]()
      {
        iscool::preferences::local_preferences& preferences =
            *m_context.get_local_preferences();

        const bool v = !haptic_feedback_enabled(preferences);
        haptic_feedback_enabled(preferences, v);
        m_controls->vibrations->set_state(v);

        m_context.get_haptic_feedback()->set_enabled(v);
      });

  m_controls->d_pad_position->connect_to_clicked(
      [this]()
      {
        iscool::preferences::local_preferences& preferences =
            *m_context.get_local_preferences();

        const bool v = !direction_pad_on_the_left(preferences);
        direction_pad_on_the_left(preferences, v);

        set_direction_pad_display(v);
      });

  m_controls->d_pad_kind->connect_to_clicked(
      [this]()
      {
        iscool::preferences::local_preferences& preferences =
            *m_context.get_local_preferences();

        const bool use_stick = !direction_pad_kind_is_stick(preferences);
        direction_pad_kind_is_stick(preferences, use_stick);

        set_stick_or_pad_display(use_stick);
      });
}

bim::axmol::app::settings_popup::~settings_popup() = default;

void bim::axmol::app::settings_popup::show()
{
  const iscool::preferences::local_preferences& preferences =
      *m_context.get_local_preferences();

  m_controls->music->set_state(music_enabled(preferences));
  m_controls->sound_effects->set_state(effects_enabled(preferences));
  m_controls->vibrations->set_state(haptic_feedback_enabled(preferences));

  set_direction_pad_display(direction_pad_on_the_left(preferences));
  set_stick_or_pad_display(direction_pad_kind_is_stick(preferences));

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
