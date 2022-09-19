# xcore hierarchical (typed) data structure

A typed data (**immutable**) structure that starts at a root ``xobject*`` and that can contain many types of data.

## Types

- bool, s8, u8, s16, u16, s32, u32, s64, u64, f32
- vector2f, vector3f, vector4f, matrix3f, matrix4f
- fileid, fileid_list
- locstr
- ``string*``
- color
- ``object*``
- ``void*`` (serialized compound, should match C++ structure layout)
- ``array<T>``, any of the above types
