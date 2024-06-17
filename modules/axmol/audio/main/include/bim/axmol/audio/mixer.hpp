#pragma once

#include <iscool/audio/platform_mixer.hpp>

namespace bim::axmol::audio
{
  class mixer : public iscool::audio::platform_mixer
  {
  public:
    mixer();

    void preload_effect(const std::string& file_path) override;

    void pause() override;
    void resume() override;

    iscool::audio::track_id
    play_effect(const std::string& file_path,
                iscool::audio::loop_mode loop) override;
    void stop_effect(iscool::audio::track_id id) override;
    void set_effects_muted(bool muted) override;

    void play_music(const std::string& file_path,
                    iscool::audio::loop_mode loop) override;
    void stop_music() override;
    void set_music_muted(bool muted) override;

  private:
    iscool::audio::track_id m_current_music_id;
    std::string m_current_music_path;
    bool m_music_muted;
    bool m_effects_muted;
  };
}
