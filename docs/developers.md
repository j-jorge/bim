# Documentation for developers

Building Bim! for the first time is as simple as launching
`./setup.sh` from the project's root directory. As long as you're in a
Linux environment with the dependencies installed.

Once the above command terminates without error, a folder
`build/linux/release` has been created. You can edit the CMake flags
there to adjust the build to your needs, or, **more importantly**, you
can run `./setup.sh --help` for additional pre-configured builds.

## Dependencies

The project handles most of its dependencies via the `setup.sh`
script, but it still needs some tools available on the system.  You'll
need CMake, a C++ compiler, and more. See
`ci/install-minimal-environment.sh` for a complete list. If you are
using Debian or Ubuntu you can run this script directly.

The dependencies handled by `setup.sh` are downloaded and built in
`.backroom/`.

## High-level project structure

The gameplay code and the server code are in `modules/bim`. The final
apps are in `apps/axmol` for the client, and `apps/server` for the
server.

# Launching the game

Supposing you've built the debug version of the game in a Linux
environment, launch the game server with the following command:

```
./build/linux/debug/apps/server/bim-server
```

Then launch the game as follows, with the environment variable set to
point to the server launched above.

```
BIM_GAME_SERVER_HOST=localhost:23899 \
  ./build/linux/debug/apps/linux/bim \
  --assets ./static-assets/ ./build/linux/debug/assets/generated/ \
  --scale 0.5
```

Pass `--help` to any program to get additional information.
