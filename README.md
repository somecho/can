![GitHub Tag](https://img.shields.io/github/v/tag/somecho/can)
# can
... opener! A multipurpose CLI program for previewing
- MIDI Files
- ... that's it for now

## Building

To build this project you will need the following
- CMake 3.30
- C++23 Compiler 
- SDL2 ([install instructions](https://trenki2.github.io/blog/2017/06/02/using-sdl2-with-cmake/))

Then all you need to run once you cloned this repo is

```cmake
cmake -B build -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build
```

## Usage

Once compiled, simply call `can file/to/open`. Since `can` is probably not yet in your path, if you are in the project root directory, you can call `./build/can FILE`. 

### MIDI Files

Calling `can` on a midifile will open a scrollable preview of a MIDI file visualized with velocity data on a piano roll grid. Try with examples provided in [`/data/midifiles`](./data/midifiles)

![image](https://github.com/user-attachments/assets/01c861f1-f468-4016-ab6c-2b1209516947)

---

© 2024 Somē Cho
