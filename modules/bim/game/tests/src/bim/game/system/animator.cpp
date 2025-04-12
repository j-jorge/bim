// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/system/animator.hpp>

#include <bim/game/animation/animation_catalog.hpp>
#include <bim/game/component/animation_state.hpp>
#include <bim/game/context/context.hpp>

#include <entt/entity/registry.hpp>

#include <gtest/gtest.h>

TEST(animator, one_animation_no_end)
{
  bim::game::context context;
  bim::game::animation_catalog& catalog =
      context.create<bim::game::animation_catalog>();
  const bim::game::animation_id anim = catalog.new_animation();

  bim::game::animation_specifications specs;
  specs.duration = std::chrono::seconds(1);
  catalog.replace_animation(anim, specs);

  EXPECT_EQ(specs.duration, catalog.get_animation(anim).duration);

  entt::registry registry;

  const entt::entity e = registry.create();
  const bim::game::animation_state& s =
      registry.emplace<bim::game::animation_state>(e, anim,
                                                   std::chrono::seconds{});
  EXPECT_EQ(anim, s.model);
  EXPECT_EQ(std::chrono::milliseconds(0), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(100));
  EXPECT_EQ(anim, s.model);
  EXPECT_EQ(std::chrono::milliseconds(100), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(400));
  EXPECT_EQ(anim, s.model);
  EXPECT_EQ(std::chrono::milliseconds(500), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(600));
  EXPECT_EQ(anim, s.model);
  EXPECT_EQ(std::chrono::milliseconds(1100), s.elapsed_time);
}

TEST(animator, dispatch_completion)
{
  bim::game::context context;
  bim::game::animation_catalog& catalog =
      context.create<bim::game::animation_catalog>();
  const bim::game::animation_id anims[2] = { catalog.new_animation(),
                                             catalog.new_animation() };
  bool anim_completed[2]{};

  entt::registry registry;
  const entt::entity e = registry.create();

  bim::game::animation_specifications specs;
  specs.duration = std::chrono::seconds(1);
  specs.next = anims[1];
  specs.dispatch_completion = [&anim_completed, e](entt::registry& r,
                                                   entt::entity entity) -> void
  {
    EXPECT_EQ(e, entity);
    EXPECT_FALSE(anim_completed[0]);
    anim_completed[0] = true;
  };
  catalog.replace_animation(anims[0], specs);

  specs.duration = std::chrono::seconds(2);
  specs.next = anims[0];
  specs.dispatch_completion = [&anim_completed, e](entt::registry& r,
                                                   entt::entity entity) -> void
  {
    EXPECT_EQ(e, entity);
    EXPECT_FALSE(anim_completed[1]);
    anim_completed[1] = true;
  };
  catalog.replace_animation(anims[1], specs);

  const bim::game::animation_state& s =
      registry.emplace<bim::game::animation_state>(e, anims[0],
                                                   std::chrono::seconds{});
  EXPECT_EQ(anims[0], s.model);
  EXPECT_FALSE(anim_completed[0]);
  EXPECT_FALSE(anim_completed[1]);
  EXPECT_EQ(std::chrono::milliseconds(0), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(100));
  EXPECT_EQ(anims[0], s.model);
  EXPECT_FALSE(anim_completed[0]);
  EXPECT_FALSE(anim_completed[1]);
  EXPECT_EQ(std::chrono::milliseconds(100), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(900));
  EXPECT_EQ(anims[1], s.model);
  EXPECT_TRUE(anim_completed[0]);
  EXPECT_FALSE(anim_completed[1]);
  EXPECT_EQ(std::chrono::milliseconds(0), s.elapsed_time);

  anim_completed[0] = false;
  bim::game::animator(context, registry, std::chrono::milliseconds(600));
  EXPECT_EQ(anims[1], s.model);
  EXPECT_FALSE(anim_completed[0]);
  EXPECT_FALSE(anim_completed[1]);
  EXPECT_EQ(std::chrono::milliseconds(600), s.elapsed_time);

  bim::game::animator(context, registry, std::chrono::milliseconds(1500));
  EXPECT_EQ(anims[0], s.model);
  EXPECT_FALSE(anim_completed[0]);
  EXPECT_TRUE(anim_completed[1]);
  EXPECT_EQ(std::chrono::milliseconds(100), s.elapsed_time);
}
