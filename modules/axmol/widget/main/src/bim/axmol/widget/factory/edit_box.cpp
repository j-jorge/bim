// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/edit_box.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/factory/scale_nine_sprite.hpp>
#include <bim/axmol/widget/label_style.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/style/declaration.hpp>

#include <axmol/ui/UIEditBox/UIEditBox.h>

bim::axmol::ref_ptr<ax::ui::EditBox>
bim::axmol::widget::factory<ax::ui::EditBox>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  const bim::axmol::ref_ptr<ax::ui::EditBox> result = ax::ui::EditBox::create(
      ax::Size(style.get_number("width", 100) * context.device_scale,
               style.get_number("height", 30) * context.device_scale),
      factory<ax::ui::Scale9Sprite>::create(
          context, *style.get_declaration("background"))
          .get());

  result->setPropagateTouchEvents(false);

  const label_style ls(context, style);

  result->setFont(ls.ttf_config.fontFilePath.c_str(), ls.ttf_config.fontSize);
  result->setFontColor(ls.color);
  result->setTextHorizontalAlignment(ls.horizontal_align);
  result->setText(ls.text.c_str());

  const iscool::optional<const std::string&> return_type_string =
      style.get_string("return-type");
  ax::ui::EditBox::KeyboardReturnType return_type =
      ax::ui::EditBox::KeyboardReturnType::DEFAULT;

  if (return_type_string)
    {
      if (*return_type_string == "done")
        return_type = ax::ui::EditBox::KeyboardReturnType::DONE;
      else if (*return_type_string == "send")
        return_type = ax::ui::EditBox::KeyboardReturnType::SEND;
      else if (*return_type_string == "search")
        return_type = ax::ui::EditBox::KeyboardReturnType::SEARCH;
      else if (*return_type_string == "go")
        return_type = ax::ui::EditBox::KeyboardReturnType::GO;
      else if (*return_type_string == "next")
        return_type = ax::ui::EditBox::KeyboardReturnType::NEXT;
      else if (*return_type_string != "default")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Invalid return type: '{}'.", *return_type_string);
    }

  result->setReturnType(return_type);

  const iscool::optional<const std::string&> input_mode_string =
      style.get_string("input-mode");
  ax::ui::EditBox::InputMode input_mode = ax::ui::EditBox::InputMode::ANY;

  if (input_mode_string)
    {
      if (*input_mode_string == "email")
        input_mode = ax::ui::EditBox::InputMode::EMAIL_ADDRESS;
      else if (*input_mode_string == "numeric")
        input_mode = ax::ui::EditBox::InputMode::NUMERIC;
      else if (*input_mode_string == "phone-number")
        input_mode = ax::ui::EditBox::InputMode::PHONE_NUMBER;
      else if (*input_mode_string == "url")
        input_mode = ax::ui::EditBox::InputMode::URL;
      else if (*input_mode_string == "decimal")
        input_mode = ax::ui::EditBox::InputMode::DECIMAL;
      else if (*input_mode_string == "single-line")
        input_mode = ax::ui::EditBox::InputMode::SINGLE_LINE;
      else if (*input_mode_string != "any")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Invalid input mode: '{}'.", *input_mode_string);
    }

  result->setInputMode(input_mode);

  const iscool::optional<const std::string&> input_flag_string =
      style.get_string("input-flag");
  ax::ui::EditBox::InputFlag input_flag =
      ax::ui::EditBox::InputFlag::INITIAL_CAPS_SENTENCE;

  if (input_flag_string)
    {
      if (*input_flag_string == "password")
        input_flag = ax::ui::EditBox::InputFlag::PASSWORD;
      else if (*input_flag_string == "sensitive")
        input_flag = ax::ui::EditBox::InputFlag::SENSITIVE;
      else if (*input_flag_string == "initial-caps-word")
        input_flag = ax::ui::EditBox::InputFlag::INITIAL_CAPS_WORD;
      else if (*input_flag_string == "initial-caps-all-characters")
        input_flag = ax::ui::EditBox::InputFlag::INITIAL_CAPS_ALL_CHARACTERS;
      else if (*input_flag_string == "lowercase-all-characters")
        input_flag = ax::ui::EditBox::InputFlag::LOWERCASE_ALL_CHARACTERS;
      else if (*input_flag_string != "initial-caps-sentence")
        ic_log(iscool::log::nature::error(), g_log_context,
               "Invalid input flag: '{}'.", *input_flag_string);
    }

  result->setInputFlag(input_flag);

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
