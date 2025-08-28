// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/popup/language_popup.hpp>

#include <bim/axmol/app/popup/message.hpp>
#include <bim/axmol/app/popup/popup.hpp>
#include <bim/axmol/app/preference/user_language.hpp>

#include <bim/axmol/widget/factory/label.hpp>
#include <bim/axmol/widget/implement_widget.hpp>
#include <bim/axmol/widget/ui/button.hpp>
#include <bim/axmol/widget/ui/list.hpp>
#include <bim/axmol/widget/ui/passive_node.hpp>

#define x_widget_scope bim::axmol::app::language_popup::
#define x_widget_type_name controls
#define x_widget_controls                                                     \
  x_widget(bim::axmol::widget::button, close_button)                          \
      x_widget(bim::axmol::widget::list, list)

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <bim/axmol/input/key_observer_handle.impl.hpp>
#include <bim/axmol/input/observer/single_key_observer.hpp>

#include <bim/axmol/find_child_by_path.hpp>

#include <iscool/i18n/gettext.hpp>
#include <iscool/language_name.hpp>
#include <iscool/preferences/local_preferences.hpp>
#include <iscool/signals/implement_signal.hpp>

#include <axmol/2d/Label.h>

namespace
{
  struct language_list_item_controls;

#define x_widget_scope
#define x_widget_type_name language_list_item_controls
#define x_widget_controls x_widget(bim::axmol::widget::button, button)

#include <bim/axmol/widget/implement_controls_struct.hpp>
}

IMPLEMENT_SIGNAL(bim::axmol::app::language_popup, reset, m_reset);

bim::axmol::app::language_popup::language_popup(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_style_bounds(*style.get_declaration("bounds"))
  , m_popup(new popup(context, *style.get_declaration("popup")))
  , m_message(
        new message_popup(context, *style.get_declaration("message-popup")))
  , m_escape(ax::EventKeyboard::KeyCode::KEY_BACK)
{
  m_inputs.push_back(m_controls->list->input_node());
  m_inputs.push_back(m_controls->close_button->input_node());
  m_inputs.push_back(m_escape);

  const auto close = [this]()
  {
    m_message_connexion.disconnect();
    m_popup->hide();
  };

  m_controls->close_button->connect_to_clicked(close);
  m_escape->connect_to_released(close);

  const iscool::style::declaration& list_item_container_style =
      *style.get_declaration("list-item");
  const iscool::style::declaration& button_item_controls =
      *style.get_declaration("list-item-controls");
  const iscool::style::declaration& button_item_bounds =
      *style.get_declaration("list-item-bounds");
  const std::string& label_path_in_button =
      *style.get_string("language-label-path");
  const iscool::language_name preferred_language =
      user_language(*context.get_local_preferences());

  for (iscool::language_name language :
       { iscool::language_name::br_FR, iscool::language_name::de_DE,
         iscool::language_name::en_GB, iscool::language_name::fr_FR,
         iscool::language_name::pt_PT, iscool::language_name::pt_BR })
    {
      language_list_item_controls controls(context.get_widget_context(),
                                           button_item_controls);
      bim::axmol::widget::button& b = *controls.button;

      ((ax::Label*)bim::axmol::find_child_by_path(b, label_path_in_button))
          ->setString(iscool::to_human_string(language));

      m_inputs.push_back(b.input_node());

      b.enable(preferred_language != language);
      b.connect_to_clicked(
          [this, language]() -> void
          {
            confirm_language(language);
          });

      const bim::axmol::ref_ptr<bim::axmol::widget::passive_node> item =
          bim::axmol::widget::factory<bim::axmol::widget::passive_node>::
              create(context.get_widget_context(), list_item_container_style);

      item->fill(controls.all_nodes, button_item_bounds);
      m_controls->list->push_back(*item);
    }
}

bim::axmol::app::language_popup::~language_popup() = default;

void bim::axmol::app::language_popup::show()
{
  m_popup->show(m_controls->all_nodes, m_style_bounds, m_inputs.root());
}

void bim::axmol::app::language_popup::confirm_language(
    iscool::language_name language)
{
  m_message_connexion = m_message->connect_to_ok(
      [this, language]() -> void
      {
        switch_to_language(language);
      });
  m_message->show_yes_no(
      ic_gettext("Do you really want to change the language setting?"));
}

void bim::axmol::app::language_popup::switch_to_language(
    iscool::language_name language) const
{
  user_language(*m_context.get_local_preferences(), language);
  m_reset();
}
