#include <bim/axmol/widget/factory/sprite.hpp>

#include <bim/axmol/widget/context.hpp>
#include <bim/axmol/widget/log_context.hpp>

#include <bim/axmol/style/apply_display.hpp>
#include <bim/axmol/style/cache.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>
#include <iscool/optional.impl.tpp>
#include <iscool/style/declaration.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/2d/SpriteFrameCache.h>

bim::axmol::ref_ptr<ax::Sprite>
bim::axmol::widget::factory<ax::Sprite>::create(
    const bim::axmol::widget::context& context,
    const iscool::style::declaration& style)
{
  bim::axmol::ref_ptr<ax::Sprite> result;
  const iscool::optional<std::string> frame_name = style.get_string("frame");

  if (!frame_name)
    ic_log(iscool::log::nature::error(), g_log_context,
           "No frame provided for sprite.");
  else
    {
      ax::SpriteFrame* const frame =
          ax::SpriteFrameCache::getInstance()->getSpriteFrameByName(
              *frame_name);

      if (frame == nullptr)
        // Assume the frame name is actually a file, e.g. backgrounds.
        result = ax::Sprite::create(*frame_name);
      else
        result = ax::Sprite::createWithSpriteFrame(frame);

      if (result == nullptr)
        ic_log(iscool::log::nature::error(), g_log_context,
               "Cannot load sprite '%s'.", *frame_name);
    }

  if (result == nullptr)
    result = ax::Sprite::create();

  bim::axmol::style::apply_display(context.style_cache.get_display(style),
                                   *result);

  return result;
}
