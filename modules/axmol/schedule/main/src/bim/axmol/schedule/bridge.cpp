#include <bim/axmol/schedule/bridge.hpp>

#include <iscool/schedule/setup.hpp>

#include <axmol/base/Director.h>
#include <axmol/base/Scheduler.h>

#include <fmt/format.h>

bim::axmol::schedule::bridge::bridge()
  : m_next_call_id(0)
  , m_key_prefix(fmt::format("{}-", (void*)this))
{
  m_call_key_buffer.reserve(m_key_prefix.size() + 10);

  iscool::schedule::initialize(std::bind(&bridge::delayed_call, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
}

bim::axmol::schedule::bridge::~bridge()
{
  ax::Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);

  iscool::schedule::finalize();
}

void bim::axmol::schedule::bridge::delayed_call(
    const std::function<void()>& f, const std::chrono::nanoseconds& delay)
{
  ax::Scheduler& scheduler(*ax::Director::getInstance()->getScheduler());

  scheduler.runOnAxmolThread(
      std::bind(&bridge::schedule_call, this, f, delay));
}

void bim::axmol::schedule::bridge::schedule_call(
    const std::function<void()>& f, const std::chrono::nanoseconds& delay)
{
  static constexpr float interval = 0;
  static constexpr unsigned int repeat = 0;

  const float delay_in_seconds =
      std::chrono::duration_cast<std::chrono::duration<float>>(delay).count();

  m_call_key_buffer = m_key_prefix;
  m_call_key_buffer += fmt::format("{}", m_next_call_id);
  ++m_next_call_id;

  ax::Director::getInstance()->getScheduler()->schedule(
      std::bind(f), this, interval, repeat, delay_in_seconds, false,
      m_call_key_buffer);
}
