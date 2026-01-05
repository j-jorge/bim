# Next

- Review the startup steps.
- Add tests for the shield: update_shield_power_ups.
- Put a ZoneScoped in all systems.

# Client

# Server

- Web services
  - Hot reload of the config.
  - Retrieve the stats.
- Keep track of the results per session and session group.

# Nice to have

- Add a timed_to_death component: dead when timer is zero.
- Set UBSAN_OPTIONS and co. to stop the tests on failure.
- Why does --console-log on server-tests only works for the first test?
- Robots, for the tests.
- Opt-in for the console-log in Android builds.
- Factorize online_game::display_bomb/flame_power_up
- Room full for named games?
- setup.sh
  - include-what-you-use
  - clang-tidy
- Flood the server with random messages.
