// SPDX-License-Identifier: AGPL-3.0-only
#include <bim/axmol/audio/mixer.hpp>

#include <iscool/audio/loop_mode.hpp>

#include <axmol/audio/AudioEngine.h>

#include <cassert>

static const iscool::audio::track_id g_invalid_audio_id =
    ax::AudioEngine::INVALID_AUDIO_ID;

bim::axmol::audio::mixer::mixer()
  : m_current_music_id(g_invalid_audio_id)
  , m_music_muted(false)
  , m_effects_muted(false)
{
  ax::AudioEngine::lazyInit();
}

void bim::axmol::audio::mixer::pause()
{
  ax::AudioEngine::pauseAll();
}

void bim::axmol::audio::mixer::resume()
{
  ax::AudioEngine::resumeAll();
}

void bim::axmol::audio::mixer::preload_effect(const std::string& file_path)
{
  ax::AudioEngine::preload(file_path);
}

iscool::audio::track_id
bim::axmol::audio::mixer::play_effect(const std::string& file_path,
                                      iscool::audio::loop_mode loop)
{
  // Handling the volume of looping effects is not implemented, so we are just
  // going to forbid loops.
  assert(loop == iscool::audio::loop_mode::once);

  return ax::AudioEngine::play2d(file_path, false, m_effects_muted ? 0 : 1);
}

void bim::axmol::audio::mixer::stop_effect(iscool::audio::track_id id)
{
  assert(id != m_current_music_id);
  ax::AudioEngine::stop(id);
}

void bim::axmol::audio::mixer::set_effects_muted(bool muted)
{
  m_effects_muted = muted;
}
void bim::axmol::audio::mixer::play_music(const std::string& file_path,
                                          iscool::audio::loop_mode loop)
{
  stop_music();

  const bool repeat_forever = loop == iscool::audio::loop_mode::forever;

  if (repeat_forever)
    m_current_music_path = file_path;

  if (!m_music_muted)
    m_current_music_id = ax::AudioEngine::play2d(file_path, repeat_forever);
}

void bim::axmol::audio::mixer::stop_music()
{
  if (m_current_music_id == g_invalid_audio_id)
    return;

  ax::AudioEngine::stop(m_current_music_id);
  m_current_music_id = g_invalid_audio_id;
  m_current_music_path.clear();
}

void bim::axmol::audio::mixer::set_music_muted(bool muted)
{
  if (m_music_muted == muted)
    return;

  if (muted)
    {
      if (m_current_music_id != g_invalid_audio_id)
        {
          ax::AudioEngine::stop(m_current_music_id);
          m_current_music_id = g_invalid_audio_id;
        }
    }
  else if (!m_current_music_path.empty())
    m_current_music_id = ax::AudioEngine::play2d(m_current_music_path, true);
}
