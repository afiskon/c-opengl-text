# c-opengl-text

On Linux you will need these packages:

```
    g++-4.8 git cmake libgl1-mesa-dev libxrandr-dev libxinerama-dev
    libxcursor-dev libglu1-mesa-dev
```

Usage:

```
    git submodule init
    git submodule update
    mkdir build
    cd build

    # on *nix:
    cmake ..
    make -j4 demo emdconv

    # on Windows:
    cmake -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -G "MinGW Makefiles" ..
    mingw32-make -j4 demo emdconv

    cd ..
    ./build/emdconv models/skybox.blend skybox.emd
    ./build/demo
```

* WASD + mouse - move camera
* M - enable/disable mouse interception
* X - enable/disable wireframes mode
* 1 - enable/disable white directional light
* 2 - enable/disable red point light
* 3 - enable/disable blue spot light
* Q - quit

Tested on Linux, MacOS and Windows.
