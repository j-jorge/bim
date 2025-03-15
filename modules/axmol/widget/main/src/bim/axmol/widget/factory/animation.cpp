// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/factory/animation.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/Animation.h>
#include <axmol/2d/SpriteFrameCache.h>

bim::axmol::ref_ptr<ax::Animation>
bim::axmol::widget::factory<ax::Animation>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  ax::Vector<ax::AnimationFrame*> animation_frames;

  for (std::size_t i = 0;; ++i)
    {
      const std::string frame_style_name = "frame." + std::to_string(i);
      const iscool::optional<const iscool::style::declaration&> frame_style =
          style.get_declaration(frame_style_name);

      if (!frame_style)
        break;

      const iscool::optional<const std::string&> frame_name =
          frame_style->get_string("name");

      if (!frame_name)
        {
          ic_log(iscool::log::nature::error(), g_log_context,
                 "Missing frame name in '{}'.", frame_style_name);
          continue;
        }

      const iscool::optional<float> frame_duration =
          frame_style->get_number("duration");

      if (!frame_duration)
        {
          ic_log(iscool::log::nature::error(), g_log_context,
                 "Missing frame duration in '{}' (name={}).", frame_style_name,
                 *frame_name);
          continue;
        }

      ax::SpriteFrame* const sprite_frame =
          ax::SpriteFrameCache::getInstance()->getSpriteFrameByName(
              *frame_name);

      if (sprite_frame == nullptr)
        {
          ic_log(iscool::log::nature::error(), g_log_context,
                 "No such animation sprite frame in '{}': {}).",
                 frame_style_name, *frame_name);
          continue;
        }

      animation_frames.pushBack(
          ax::AnimationFrame::create(sprite_frame, *frame_duration, {}));
    }

  return ax::Animation::create(animation_frames, 1,
                               style.get_number("loops", 1));
}
