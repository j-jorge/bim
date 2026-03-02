// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/game/game_state_serialization.hpp>

#include <bim/game/all_components.hpp>

#include <bim/tracy.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

#include <algorithm>
#include <type_traits>

namespace
{
  class input_archive
  {
  public:
    explicit input_archive(const char* storage)
      : m_cursor(storage)
    {}

    template <typename T>
    void operator()(T& value)
    {
      char* data = reinterpret_cast<char*>(std::addressof(value));
      std::copy(m_cursor, m_cursor + sizeof(T), data);
      m_cursor += sizeof(T);
    }

    template <typename T>
    void operator()(entt::entity& entity, T& value)
    {
      operator()(entity);
      operator()(value);
    }

  private:
    const char* m_cursor;
  };
}

namespace
{
  class output_archive
  {
  public:
    output_archive(bim::game::archive_storage& storage)
      : m_storage(storage)
    {}

    template <typename T>
    void operator()(const T& value) const
    {
      const char* data = reinterpret_cast<const char*>(std::addressof(value));
      constexpr std::size_t n = sizeof(T);
      m_storage.insert(m_storage.end(), data, data + n);
    }

    template <typename T>
    void operator()(entt::entity entity, const T& value) const
    {
      operator()(entity);
      operator()(value);
    }

  private:
    bim::game::archive_storage& m_storage;
  };
}

template <typename Archive, typename Snapshot>
static void archive_io(Snapshot&& snapshot, Archive&& archive)
{
  snapshot.template get<entt::entity>(archive)
#define bim_game_x_component(n) .template get<bim::game::n>(archive)
#include <bim/game/for_each_component.hpp>
      ;
}

void bim::game::serialize_state(archive_storage& storage,
                                const entt::registry& registry)
{
  ZoneScoped;

  storage.clear();

  archive_io(entt::snapshot(registry), output_archive(storage));
}

void bim::game::deserialize_state(entt::registry& registry,
                                  const archive_storage& storage)
{
  ZoneScoped;

  registry.clear();

  archive_io(entt::snapshot_loader(registry), input_archive(storage.data()));
}
