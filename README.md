# charon, game data (WIP)

Provides a class called `bigfile_t` that acts as a game data archive from which you can load game data by the use of `fileid_t`.
You can have many `bigfiles` on disk, and you can load their TOC (table of contents) into memory and then load data from the `bigfile`.
A `bigfile` has an index as an identifier and a `fileid_t` is a 64-bit identifier that maps to a `bigfile` and a file in that `bigfile`.

You can imagine that you have compiled many resources, like textures, models, materials, shaders, sounds, etc. into a `bigfile` and you can 
load them into memory and use them in your game.

Bigfile(s) can be generated using ``https://github.com/jurgen-kluft/BuildSystem.Core``.

## types

- bool, s8, u8, s16, u16, s32, u32, s64, u64, f32
- fileid_t
- locstr_t
- string_t
- color_t
- ``void*`` (serialized compound, should match C++ structure layout)
- ``array<T>`` (any of the above types)
