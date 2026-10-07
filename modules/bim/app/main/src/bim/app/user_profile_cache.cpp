// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/app/user_profile_cache.hpp>

#include <bim/app/business/user_profile.hpp>
#include <bim/app/job/fetch_user_profiles_job.hpp>

#include <iscool/schedule/delayed_call.hpp>
#include <iscool/signals/signal.impl.tpp>

#include <vector>

std::size_t bim::app::user_profile_cache::user_profile_hash::operator()(
    const user_profile& p) const
{
  return boost::hash<bim::net::user_id>()(p.user_id);
}

std::size_t bim::app::user_profile_cache::user_profile_hash::operator()(
    bim::net::user_id id) const
{
  return boost::hash<bim::net::user_id>()(id);
}

bool bim::app::user_profile_cache::user_profile_equals::operator()(
    const user_profile& lhs, const user_profile& rhs) const
{
  return lhs.user_id == rhs.user_id;
}

bool bim::app::user_profile_cache::user_profile_equals::operator()(
    bim::net::user_id lhs, const user_profile& rhs) const
{
  return lhs == rhs.user_id;
}

bim::app::user_profile_cache::user_profile_cache(
    analytics_service& analytics, const bim::business::request_headers& r)
  : m_job(new fetch_user_profiles_job(analytics, r))
{
  m_job->connect_to_done(
      [this](std::span<user_profile> profiles)
        {
          insert_profiles(profiles);
        });
}

bim::app::user_profile_cache::~user_profile_cache() = default;

void bim::app::user_profile_cache::invalidate(bim::net::user_id id)
{
  m_profiles.erase(id);
  m_signals.erase(id);
}

const bim::app::user_profile*
bim::app::user_profile_cache::get(bim::net::user_id id) const
{
  const profile_set::const_iterator it = m_profiles.find(id);

  if (it == m_profiles.end())
    return nullptr;

  return &*it;
}

iscool::signals::connection bim::app::user_profile_cache::fetch(
    bim::net::user_id id, std::function<void(const user_profile&)> ready)
{
  if (std::find(m_to_be_fetched.begin(), m_to_be_fetched.end(), id)
      == m_to_be_fetched.end())
    {
      m_to_be_fetched.push_back(id);

      if ((m_to_be_fetched.size() == 1) && !m_job_start_connection.connected())
        m_job_start_connection = iscool::schedule::delayed_call(
            [this]()
              {
                fetch_profiles();
              });
    }

  return m_signals[id].connect(std::move(ready));
}

void bim::app::user_profile_cache::fetch_profiles()
{
  m_job->start(m_to_be_fetched);
  m_to_be_fetched.clear();
}

void bim::app::user_profile_cache::insert_profiles(
    std::span<user_profile> profiles)
{
  std::vector<bim::net::user_id> ids;
  ids.reserve(profiles.size());

  for (user_profile& p : profiles)
    {
      ids.push_back(p.user_id);
      m_profiles.insert(std::move(p));
    }

  for (const bim::net::user_id id : ids)
    {
      const signal_map::const_iterator it = m_signals.find(id);

      if (it != m_signals.end())
        it->second(*m_profiles.find(id));
    }
}
