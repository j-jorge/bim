# Next

- Display the game features in the output of bim-player.
- Review the client's startup steps.
- Visual bug for stats with zero victories.
- Share the logs when sending an e-mail.
- Display the host in the debug menu.
- Game state's CRC to be checked by the server.

# Client

# Server

- Web services
  - Hot reload of the config.
  - Retrieve the stats.
- Keep track of the results per session and session group.

# Nice to have

- Compute the client's ping on the server, use it to handle disconnections.
- Move the context out of the contest. It's the same for every contest.
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
