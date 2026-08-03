# Next

## Server
- Inform the business server about the played games.

## Client
- Send the stored inventory to the business server on start.
- Get the player's inventory from the business server.
- Fetch the game reward from the business server.
- Allow to edit the player's name.
- Display the opponents' names on the matchmaking screen.

# Nice to have

- Hot reload of the config on the server.
- Do not update the fog if there is no fog.
- Standalone application with a bot playing online.
- Stress-test application to launch many clients connecting to the
  business server and playing games in loops.
- Compute the client's ping on the server, use it to handle disconnections.
- Move the context out of the contest. It's the same for every contest.
- Why does --console-log on server-tests only works for the first test?
- Opt-in for the console-log in Android builds.
- Room full for named games?
- setup.sh
  - include-what-you-use
- Bot improvement: don't pick the invisibility if the bot is already invisible.
