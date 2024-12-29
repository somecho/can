![GitHub Tag](https://img.shields.io/github/v/tag/somecho/can)
# can
... opener! A multipurpose CLI program for previewing
- MIDI Files
- ... that's it for now

## Building

To build this project you will need the following
- git
- CMake 3.30
- A C++23 capable compiler

Then all you need to run once you cloned this repo is

```cmake
cmake -B build -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build
```

## Usage

Once compiled, simply call `can file/to/open`. Since `can` is probably not yet in your path, if you are in the project root directory, you can call `./build/can FILE`. 

### MIDI Files

Calling `can` on a midifile will open a scrollable preview of a MIDI file visualized with velocity data on a piano roll grid. Try with examples provided in [`/data/midifiles`](./data/midifiles). Some screenshots: 

![image](https://github.com/user-attachments/assets/c9edea2f-3ada-42e7-a9b3-dc95fcc8c532)
![image](https://github.com/user-attachments/assets/a6550b5b-993a-4791-848f-fd6dbecd89f0)



---

© 2024 Somē Cho
