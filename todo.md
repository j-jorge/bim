# Next

- Shield feature.
- Local stats on the lobby.
- Game feature submenu.

# Client

- At most three enabled features per game.
- Display the frame duration (without the sleep) in addition to the FPS.
- Close the settings when the back button is pressed.

# Server

- Collect some stats about the games and clients.
- Web services
  - Hot reload of the config.
  - Retrieve the stats.
- Keep track of the results per session and session group.

# Nice to have

- Set UBSAN_OPTIONS and co. to stop the tests on failure.
- Why does --console-log on server-tests only works for the first test?
- Robots, for the tests.
- Opt-in for the console-log in Android builds.
- Tests for the factories.
- Tests for the flame power-ups.
- Display some stats about the server in the client (e.g. active player
  count).
- Factorize online_game::display_bomb/flame_power_up
- Factorize flame/bomb_power_up_spawners
- Room full for named games?
- Tests for contest::arena setters.
- Copy arena without alloc.
- setup.sh
  - include-what-you-use
  - clang-tidy
- Flood the server with random messages.
