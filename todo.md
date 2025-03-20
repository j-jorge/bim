# Next

- Use a version.minor scheme for the versions.
- Unlock system for the game features, per game count.
- Display a message when a new version is available.
- Pick the game server according to the app version, not the protocol version.

# General

- Shield feature.

# Client

- Automatically launch the game when one player is ready.
- Add a "shop" where the player can donate to the devs.
- Get some coins for each game, unlock the features with the coins.
- At most three enabled features per game.
- Display the frame duration (without the sleep) in addition to the FPS.

# Server

- Load the config from a Json file.
- Accept config parameters as command line arguments.
- Collect some stats about the games and clients.
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
