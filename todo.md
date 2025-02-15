# Next

- Check the soft stick, it has been reported to not react.
- Automatically launch the game when one player is ready.
- Restart the app if the player leaves for 30 seconds or more.
- Fog feature.
- Shield feature.
- Nice UI for the in-game screen.
- Add a contact button in the settings.
- Remove the sound from the bomb button.
- Display the frame duration (without the sleep) in addition to the FPS.

# Client

- Add a "shop" where the player can donate to the devs.
- Get some coins for each game, unlock the features with the coins.
- At most three enabled features per game.
- Remove the build of the bundle when only the APK is targeted.

# Server

- Load the config from a Json file.
- Accept config parameters as command line arguments.
- Collect some stats about the games and clients.
- Refresh session on every message.
- Web services
  - Hot reload of the config.
  - Retrieve the stats.
- Keep track of the results per session and session group.

# Nice to have

- Why does --console-log on server-tests only works for the first test?
- Robots, for the tests.
- Opt-in for the console-log in Android builds.
- Rewrite the control area on the in-game screen.
- Tests for the factories.
- Tests for the flame power-ups.
- Display some stats about the server in the client (e.g. active player
  count).
- Factorize online_game::display_bomb/flame_power_up
- Factorize flame/bomb_power_up_factory
- Factorize flame/bomb_power_up_spawners
- Room full for named games?
- Tests for contest::arena setters.
- Copy arena without alloc.
- setup.sh
  - include-what-you-use
  - clang-tidy
- Flood the server with random messages.
