// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/widget/animation/animation_cache.hpp>

#include <bim/axmol/widget/animation/animation.hpp>

#include <iscool/json/cast_bool.hpp>
#include <iscool/json/cast_string.hpp>
#include <iscool/json/cast_uint.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/json/is_of_type_bool.hpp>
#include <iscool/json/is_of_type_string.hpp>
#include <iscool/json/is_of_type_uint.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/error.hpp>

#include <axmol/2d/Sprite.h>
#include <axmol/2d/SpriteFrameCache.h>

bim::axmol::widget::animation_cache::animation_cache() = default;
bim::axmol::widget::animation_cache::~animation_cache() = default;

void bim::axmol::widget::animation_cache::load(
    const std::string_view& json_path)
{
  const Json::Value config = iscool::json::from_file(std::string(json_path));

  if (!config.isObject())
    {
      ic_log(iscool::log::nature::error(), "animation_cache",
             "Failed to load animations '{}'.", json_path);
      return;
    }

  load(config);
}

void bim::axmol::widget::animation_cache::load(const Json::Value& config)
{
  ax::SpriteFrameCache& sprite_frame_cache =
      *ax::SpriteFrameCache::getInstance();

  for (Json::Value::const_iterator it = config.begin(), eit = config.end();
       it != eit; ++it)
    {
      const std::string key = iscool::json::cast<std::string>(it.key());

      if (!it->isObject())
        {
          ic_log(iscool::log::nature::error(), "animation_cache",
                 "Bad value for '{}'.", key);
          continue;
        }

      const Json::Value& value = *it;
      std::chrono::milliseconds default_duration{};

      const Json::Value* const json_duration = value.find("frame_duration.ms");

      if (json_duration)
        {
          if (!iscool::json::is_of_type<std::uint32_t>(*json_duration))
            {
              ic_log(iscool::log::nature::error(), "animation_cache",
                     "Duration is not a uint32_t '{}'.", key);
              continue;
            }

          default_duration = std::chrono::milliseconds(
              iscool::json::cast<std::uint32_t>(*json_duration));
        }

      bool default_flip_x = false;
      const Json::Value* const flip_x = value.find("flip.x");

      if (flip_x)
        {
          if (!iscool::json::is_of_type<bool>(*flip_x))
            {
              ic_log(iscool::log::nature::error(), "animation_cache",
                     "X-axis Flip flag is not a boolean '{}'.", key);
              continue;
            }

          default_flip_x = iscool::json::cast<bool>(*flip_x);
        }

      const Json::Value* const frames = value.find("frames");

      if (!frames)
        {
          ic_log(iscool::log::nature::error(), "animation_cache",
                 "Missing frames in '{}'.", key);
          continue;
        }

      std::chrono::milliseconds display_date{};
      animation a;

      for (Json::Value::const_iterator fit = frames->begin(),
                                       feit = frames->end();
           fit != feit; ++fit)
        {
          const Json::Value& json_frame = *fit;
          std::string sprite_frame_name;
          animation::frame animation_frame;

          bool frame_flip_x = default_flip_x;

          if (iscool::json::is_of_type<std::string>(json_frame))
            sprite_frame_name = iscool::json::cast<std::string>(json_frame);
          else if (!json_frame.isObject())
            {
              ic_log(iscool::log::nature::error(), "animation_cache",
                     "Unexpected type for frame #{} in {}.", fit.index(), key);
              continue;
            }
          else
            {
              const Json::Value* const frame_name = json_frame.find("name");

              if (!frame_name
                  || !iscool::json::is_of_type<std::string>(*frame_name))
                {
                  ic_log(iscool::log::nature::error(), "animation_cache",
                         "Missing or non-string name for frame #{} in {}.",
                         fit.index(), key);
                  continue;
                }

              sprite_frame_name = iscool::json::cast<std::string>(*frame_name);

              const Json::Value* const flip_x = json_frame.find("flip.x");

              if (flip_x)
                {
                  if (!iscool::json::is_of_type<bool>(*flip_x))
                    {
                      ic_log(iscool::log::nature::error(), "animation_cache",
                             "X-axis Flip flag is not a boolean in frame #{} "
                             "in {}.",
                             fit.index(), key);
                      continue;
                    }

                  frame_flip_x = iscool::json::cast<bool>(*flip_x);
                }
            }

          ax::SpriteFrame* const sprite_frame =
              sprite_frame_cache.findFrame(sprite_frame_name);

          if (!sprite_frame)
            ic_log(iscool::log::nature::error(), "animation_cache",
                   "Could not load frame name '{}' in frame #{} "
                   "from '{}'.",
                   sprite_frame_name, fit.index(), key);

          animation_frame.sprite_frame = sprite_frame;
          animation_frame.display_date = display_date;
          animation_frame.flip_x = frame_flip_x;
          display_date += default_duration;
          a.frames.push_back(animation_frame);
        }

      std::chrono::milliseconds total_duration = display_date;
      const Json::Value* const loop = value.find("loop");

      if (loop)
        {
          if (!iscool::json::is_of_type<bool>(*loop))
            {
              ic_log(iscool::log::nature::error(), "animation_cache",
                     "Loop flag is not a boolean '{}'.", key);
              continue;
            }

          if (!iscool::json::cast<bool>(*loop))
            total_duration = {};
        }

      a.total_duration = total_duration;
      m_animations.emplace(key, std::move(a));
    }
}

const bim::axmol::widget::animation&
bim::axmol::widget::animation_cache::get(const std::string_view& name) const
{
  assert(m_animations.find(name) != m_animations.end());
  return m_animations.find(name)->second;
}
