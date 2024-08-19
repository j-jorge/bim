// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/scale_nine_sprite.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/SpriteFrameCache.h>
#include <axmol/ui/UIScale9Sprite.h>

bim::axmol::ref_ptr<ax::ui::Scale9Sprite>
bim::axmol::widget::factory<ax::ui::Scale9Sprite>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  bim::axmol::ref_ptr<ax::ui::Scale9Sprite> result;
  const iscool::optional<std::string> frame_name = style.get_string("frame");

  if (!frame_name)
    ic_log(iscool::log::nature::error(), g_log_context,
           "No frame provided for 9-slices.");
  else
    {
      ax::SpriteFrame* const frame =
          ax::SpriteFrameCache::getInstance()->getSpriteFrameByName(
              *frame_name);

      if (frame == nullptr)
        ic_log(iscool::log::nature::error(), g_log_context,
               "Cannot load sprite '%s'.", *frame_name);
      else
        {
          ax::Vec2 third_of_size = frame->getRect().size / 3;

          if (frame->isRotated())
            std::swap(third_of_size.x, third_of_size.y);

          const ax::Rect insets(
              style.get_number("insets.left", third_of_size.x),
              style.get_number("insets.top", third_of_size.y),
              style.get_number("insets.width", third_of_size.x),
              style.get_number("insets.height", third_of_size.y));

          result = ax::ui::Scale9Sprite::createWithSpriteFrame(frame, insets);
        }
    }

  if (result == nullptr)
    result = ax::ui::Scale9Sprite::create();

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
