# cgamedata, hierarchical data structure

A typed data (**immutable**) structure that starts at a root ``object_t*`` and that can contain many types of data.
This data can be generated using ``https://github.com/jurgen-kluft/BuildSystem.Core``.

## types

- bool, s8, u8, s16, u16, s32, u32, s64, u64, f32
- vec2f, vec3f, vec4f, mat3f, mat4f
- fileid_t
- locstr_t
- ``string*``
- color_t
- ``object_t*``
- ``void*`` (serialized compound, should match C++ structure layout)
- ``array<T>`` (any of the above types)
