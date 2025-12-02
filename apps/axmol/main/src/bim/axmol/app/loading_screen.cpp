// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/app/loading_screen.hpp>

#include <bim/axmol/app/application_event_dispatcher.hpp>
#include <bim/axmol/app/main_scene.hpp>

#include <bim/axmol/widget/add_group_as_children.hpp>
#include <bim/axmol/widget/apply_actions.hpp>
#include <bim/axmol/widget/apply_bounds.hpp>

#include <bim/app/preference/user_language.hpp>

#include <bim/tracy.hpp>

#define x_widget_scope bim::axmol::app::loading_screen::
#define x_widget_type_name controls
#define x_widget_controls

#include <bim/axmol/widget/implement_controls_struct.hpp>

#include <iscool/files/file_exists.hpp>
#include <iscool/files/read_file.hpp>
#include <iscool/i18n/load_translations.hpp>
#include <iscool/json/from_file.hpp>
#include <iscool/log/log.hpp>
#include <iscool/log/nature/warning.hpp>
#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/implement_signal.hpp>
#include <iscool/style/loader.hpp>

#include <axmol/2d/SpriteFrameCache.h>
#include <axmol/base/Director.h>
#include <axmol/renderer/Shaders.h>
#include <axmol/renderer/TextureCache.h>
#include <axmol/renderer/backend/ProgramManager.h>

#if BIM_ENABLE_TRACY
static const char* const g_tracy_tag_locked = "locked";
static const char* const g_tracy_tag_loading = "loading";
static const char* const g_tracy_tag_load_textures = "textures";
#endif

IMPLEMENT_SIGNAL(bim::axmol::app::loading_screen, done, m_done);

bim::axmol::app::loading_screen::loading_screen(
    const context& context, const iscool::style::declaration& style)
  : m_context(context)
  , m_container(ax::Node::create())
  , m_controls(context.get_widget_context(), *style.get_declaration("widgets"))
  , m_bounds(*style.get_declaration("bounds"))
  , m_action_done(*style.get_declaration("action.done"))
{}

bim::axmol::app::loading_screen::~loading_screen() = default;

void bim::axmol::app::loading_screen::start()
{
  FrameMarkStart(g_tracy_tag_locked);

  m_context.get_main_scene()->add_in_overlays(*m_container, m_inputs.root());
  bim::axmol::widget::add_group_as_children(*m_container,
                                            m_controls->all_nodes);
  bim::axmol::widget::apply_bounds(m_context.get_widget_context(),
                                   m_controls->all_nodes, m_bounds);

  m_start_connection = iscool::schedule::delayed_call(
      [this]()
      {
        load_resources();
      });
}

void bim::axmol::app::loading_screen::stop()
{
  if (!m_container->isRunning())
    return;

  bim::axmol::widget::apply_actions(m_action_runner,
                                    m_context.get_widget_context(),
                                    m_controls->all_nodes, m_action_done,
                                    [this]()
                                    {
                                      stopped();
                                    });
}

void bim::axmol::app::loading_screen::load_resources()
{
  FrameMarkStart(g_tracy_tag_loading);

  m_pending_resources = 1;

  m_resources = iscool::json::from_file("resources.json");

  load_textures();

  load_translations();
  load_shaders();
  load_styles();

  one_loaded_resource();
}

void bim::axmol::app::loading_screen::load_translations()
{
  ZoneScoped;

  std::string translations_file;

  const auto check_language_code = [&](std::string_view c) -> bool
  {
    translations_file = "i18n/";
    translations_file += c;
    translations_file += ".mo";
    return iscool::files::file_exists(translations_file);
  };

  const iscool::language_name language =
      bim::app::user_language(*m_context.get_local_preferences());

  if (!check_language_code(iscool::to_string(language))
      && !check_language_code(
          iscool::to_string(iscool::to_language_code(language))))
    translations_file = "i18n/en.mo";

  const std::unique_ptr<std::istream> mo_file =
      iscool::files::read_file(translations_file);

  if (!iscool::i18n::load_translations(language, *mo_file))
    ic_log(iscool::log::nature::warning(), "loading",
           "Could not read translations from {}.", translations_file);
}

void bim::axmol::app::loading_screen::load_textures()
{
  FrameMarkStart(g_tracy_tag_load_textures);

  ax::TextureCache& cache = *ax::Director::getInstance()->getTextureCache();
  const Json::Value& textures = m_resources["textures"];

  m_pending_textures = textures.size();
  ++m_pending_resources;

  for (const Json::Value& r : textures)
    cache.addImageAsync(r.asString(),
                        [this](ax::Texture2D*)
                        {
                          one_loaded_texture();
                        });
}

void bim::axmol::app::loading_screen::load_sprite_sheets()
{
  ZoneScoped;

  ax::SpriteFrameCache& cache = *ax::SpriteFrameCache::getInstance();

  for (const Json::Value& r : m_resources["sprite-sheets"])
    cache.addSpriteFramesWithFile(r.asString());
}

void bim::axmol::app::loading_screen::load_shaders()
{
  ZoneScoped;

  ax::backend::ProgramManager& manager =
      *ax::backend::ProgramManager::getInstance();

  for (const Json::Value& r : m_resources["shaders"])
    manager.registerCustomProgram(ax::positionTextureColor_vert, r.asString(),
                                  ax::VertexLayoutType::Sprite);
}

void bim::axmol::app::loading_screen::load_styles()
{
  ZoneScoped;

  for (const Json::Value& r : m_resources["styles"])
    iscool::style::loader::load(r.asString());
}

void bim::axmol::app::loading_screen::one_loaded_texture()
{
  assert(m_pending_textures > 0);
  --m_pending_textures;

  if (m_pending_textures == 0)
    {
      FrameMarkEnd(g_tracy_tag_load_textures);

      load_sprite_sheets();
      one_loaded_resource();
    }
}

void bim::axmol::app::loading_screen::one_loaded_resource()
{
  assert(m_pending_resources > 0);
  --m_pending_resources;

  if (m_pending_resources == 0)
    {
      m_resources = {};

      FrameMarkEnd(g_tracy_tag_loading);
      m_done();
    }
}

void bim::axmol::app::loading_screen::stopped()
{
  assert(m_container->isRunning());
  m_context.get_main_scene()->remove_from_overlays(*m_container);
  m_context.get_event_dispatcher()->dispatch("loaded");

  FrameMarkEnd(g_tracy_tag_locked);
}
