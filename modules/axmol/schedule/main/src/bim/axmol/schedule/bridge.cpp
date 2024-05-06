#include <bim/axmol/schedule/bridge.hpp>

#include <iscool/schedule/setup.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/Scheduler.h>

#include <algorithm>

class bim::axmol::schedule::bridge::scheduler_target_node : public ax::Node
{
public:
  explicit scheduler_target_node(bim::axmol::schedule::bridge& bridge)
    : m_bridge(bridge)
  {}

  scheduler_target_node(const scheduler_target_node&) = delete;
  scheduler_target_node& operator=(const scheduler_target_node&) = delete;

private:
  void update(float dt) override
  {
    m_bridge.tick();
  }

private:
  bim::axmol::schedule::bridge& m_bridge;
};

bim::axmol::schedule::bridge::bridge()
  : m_last_tick_date(0)
  , m_target((ax::Node*)new scheduler_target_node(*this))

{
  constexpr int capacity = 16;
  m_scheduled_delays_ns.reserve(capacity);
  m_scheduled_functions.reserve(capacity);
  m_scheduled_functions_scratch.reserve(capacity);

  iscool::schedule::initialize(std::bind(&bridge::delayed_call, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));

  constexpr int priority = -1;
  constexpr bool paused = false;
  ax::Director::getInstance()->getScheduler()->scheduleUpdate(
      m_target.get(), priority, paused);
}

bim::axmol::schedule::bridge::~bridge()
{
  ax::Director::getInstance()->getScheduler()->unscheduleAllForTarget(
      m_target.get());

  iscool::schedule::finalize();
}

void bim::axmol::schedule::bridge::delayed_call(
    const std::function<void()>& f, const std::chrono::nanoseconds& delay)
{
  assert(delay.count() <= std::numeric_limits<std::uint32_t>::max());

  const std::uint32_t delay_int =
      (delay.count() <= 17'000'000) ? 1 : delay.count();
  const std::unique_lock<std::mutex> lock(m_mutex);

  const std::size_t i = std::ranges::find_if(m_scheduled_delays_ns,
                                             [=](std::uint32_t d)
                                             {
                                               return d >= delay_int;
                                             })
                        - m_scheduled_delays_ns.begin();

  const std::int64_t schedule_date =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count()
      + delay.count();

  auto trace = [=]()
  {
    const std::int64_t call_date =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
    const float d =
        std::chrono::duration_cast<std::chrono::duration<float>>(delay)
            .count();

    if (std::abs(call_date - schedule_date) >= 1'000'000)
      printf("BAD Calling with offset of %ld ms. (scheduled for %ld with "
             "delay %f, now is "
             "%ld).\n",
             (call_date - schedule_date) / 1000000, schedule_date, d,
             call_date);
    else
      printf("OK Calling with offset of %ld ns. (scheduled for %ld, now is "
             "%ld).\n",
             call_date - schedule_date, schedule_date, call_date);

    f();
  };

  m_scheduled_delays_ns.insert(m_scheduled_delays_ns.begin() + i, delay_int);
  m_scheduled_functions.insert(m_scheduled_functions.begin() + i, trace);

  // printf("schedule:");
  // for (std::size_t i = 0; i != m_scheduled_delays_ns.size(); ++i)
  //   printf(" %.9u", m_scheduled_delays_ns[i]);
  // printf("\n");

  assert(m_scheduled_delays_ns.size() == m_scheduled_functions.size());
  assert(std::ranges::is_sorted(m_scheduled_delays_ns));
}

void bim::axmol::schedule::bridge::tick()
{
  const std::chrono::nanoseconds now =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch());

  // const std::chrono::nanoseconds dt_ns = now - m_last_tick_date;
  const std::chrono::nanoseconds dt_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::duration<std::size_t, std::ratio<1, 60>>(1));
  assert(dt_ns.count() <= std::numeric_limits<std::uint32_t>::max());

  const std::uint32_t dt_int = dt_ns.count();

  {
    const std::unique_lock<std::mutex> lock(m_mutex);

    const std::size_t split = std::ranges::find_if(m_scheduled_delays_ns,
                                                   [=](std::uint32_t d)
                                                   {
                                                     return d > dt_int;
                                                   })
                              - m_scheduled_delays_ns.begin();

    // if (!m_scheduled_delays_ns.empty())
    //   {
    //     printf("tick by %f (%u ns.):", dt, dt_int);
    //     for (std::size_t i = 0; i != split; ++i)
    //       printf(" %.9u", m_scheduled_delays_ns[i]);
    //     printf(" | ");
    //     for (std::size_t i = split; i != m_scheduled_delays_ns.size(); ++i)
    //       printf(" %.9u", m_scheduled_delays_ns[i]);
    //     printf("\n");
    //   }
    const auto selection_start = m_scheduled_functions.begin();
    const auto selection_end = selection_start + split;

    m_scheduled_functions_scratch.resize(split);
    std::ranges::move(selection_start, selection_end,
                      m_scheduled_functions_scratch.begin());
    m_scheduled_functions.erase(selection_start, selection_end);
    m_scheduled_delays_ns.erase(m_scheduled_delays_ns.begin(),
                                m_scheduled_delays_ns.begin() + split);

    for (std::uint32_t& d : m_scheduled_delays_ns)
      d -= dt_int;

    assert(std::ranges::is_sorted(m_scheduled_delays_ns));
    // printf("tick:");
    // for (std::size_t i = 0; i != m_scheduled_delays_ns.size(); ++i)
    //   printf(" %.9u", m_scheduled_delays_ns[i]);
    // printf("\n");
    assert(m_scheduled_delays_ns.size() == m_scheduled_functions.size());
  }

  for (const std::function<void()>& f : m_scheduled_functions_scratch)
    f();
}
