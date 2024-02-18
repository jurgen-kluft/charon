#include "ccore/c_target.h"
#include "cbase/c_hash.h"
#include "cbase/c_log.h"
#include "cbase/c_va_list.h"

#include "cgamedata/c_object.h"

namespace ncore
{
    namespace ngd
    {
        static const s32 sZeroMemory[]   = {0, 0, 0, 0};
        const vec3f_t    vec3f_t::sZero  = *((vec3f_t *)sZeroMemory);
        const vec3fx_t   vec3fx_t::sZero = *((vec3fx_t *)sZeroMemory);

        class member_type_t
        {
        public:
            enum __enum
            {
                TYPE_ARRAY = 0x80,
                TYPE_PTR   = 0x40,

                TYPE_BOOL8  = 1,
                TYPE_INT8   = 2,
                TYPE_INT16  = 3,
                TYPE_INT32  = 4,
                TYPE_INT64  = 5,
                TYPE_UINT8  = 6,
                TYPE_UINT16 = 7,
                TYPE_UINT32 = 8,
                TYPE_UINT64 = 9,

                TYPE_FX16 = 10,
                TYPE_FX32 = 11,

                TYPE_FLOAT = 12,

                TYPE_STRING = 13,
                TYPE_LOCSTR = 14,
                TYPE_FILEID = 15,
                TYPE_OBJECT = 16,
                TYPE_COLOR  = 17,

                TYPE_VECTOR3F  = 18,
                TYPE_VECTOR3FX = 19,

                TYPE_BOOL8_ARRAY  = TYPE_BOOL8 | TYPE_ARRAY,
                TYPE_INT8_ARRAY   = TYPE_INT8 | TYPE_ARRAY,
                TYPE_INT16_ARRAY  = TYPE_INT16 | TYPE_ARRAY,
                TYPE_INT32_ARRAY  = TYPE_INT32 | TYPE_ARRAY,
                TYPE_INT64_ARRAY  = TYPE_INT64 | TYPE_ARRAY,
                TYPE_UINT8_ARRAY  = TYPE_UINT8 | TYPE_ARRAY,
                TYPE_UINT16_ARRAY = TYPE_UINT16 | TYPE_ARRAY,
                TYPE_UINT32_ARRAY = TYPE_UINT32 | TYPE_ARRAY,
                TYPE_UINT64_ARRAY = TYPE_UINT64 | TYPE_ARRAY,

                TYPE_FX16_ARRAY = TYPE_FX16 | TYPE_ARRAY,
                TYPE_FX32_ARRAY = TYPE_FX32 | TYPE_ARRAY,

                TYPE_FLOAT_ARRAY = TYPE_FLOAT | TYPE_ARRAY,

                TYPE_STRING_ARRAY    = TYPE_STRING | TYPE_ARRAY,
                TYPE_LOCSTR_ARRAY    = TYPE_LOCSTR | TYPE_ARRAY,
                TYPE_FILEID_ARRAY    = TYPE_FILEID | TYPE_ARRAY,
                TYPE_OBJECT_ARRAY    = TYPE_OBJECT | TYPE_ARRAY,
                TYPE_COLOR_ARRAY     = TYPE_COLOR | TYPE_ARRAY,
                TYPE_VECTOR3F_ARRAY  = TYPE_VECTOR3F | TYPE_ARRAY,
                TYPE_VECTOR3FX_ARRAY = TYPE_VECTOR3FX | TYPE_ARRAY,

                TYPE_MAX,
            };

            inline bool isArray() const { return (m_type & TYPE_ARRAY) == TYPE_ARRAY; }
            inline bool isPtr() const { return (m_type & TYPE_PTR) == TYPE_PTR; }

            inline bool operator==(__enum e) const { return m_type == e; }

            u8 m_type;
        };

        static member_type_t MemberTypeArray[member_type_t::TYPE_MAX] = {
          {member_type_t::TYPE_BOOL8},        {member_type_t::TYPE_INT8},           {member_type_t::TYPE_INT16},           {member_type_t::TYPE_INT32},        {member_type_t::TYPE_INT64},
          {member_type_t::TYPE_UINT8},        {member_type_t::TYPE_UINT16},         {member_type_t::TYPE_UINT32},          {member_type_t::TYPE_UINT64},       {member_type_t::TYPE_FX16},
          {member_type_t::TYPE_FX32},         {member_type_t::TYPE_FLOAT},          {member_type_t::TYPE_STRING},          {member_type_t::TYPE_LOCSTR},       {member_type_t::TYPE_FILEID},
          {member_type_t::TYPE_OBJECT},       {member_type_t::TYPE_COLOR},          {member_type_t::TYPE_VECTOR3F},        {member_type_t::TYPE_VECTOR3FX},    {member_type_t::TYPE_BOOL8_ARRAY},
          {member_type_t::TYPE_INT8_ARRAY},   {member_type_t::TYPE_INT16_ARRAY},    {member_type_t::TYPE_INT32_ARRAY},     {member_type_t::TYPE_INT64_ARRAY},  {member_type_t::TYPE_UINT8_ARRAY},
          {member_type_t::TYPE_UINT16_ARRAY}, {member_type_t::TYPE_UINT32_ARRAY},   {member_type_t::TYPE_UINT64_ARRAY},    {member_type_t::TYPE_FX16_ARRAY},   {member_type_t::TYPE_FX32_ARRAY},
          {member_type_t::TYPE_FLOAT_ARRAY},  {member_type_t::TYPE_STRING_ARRAY},   {member_type_t::TYPE_LOCSTR_ARRAY},    {member_type_t::TYPE_FILEID_ARRAY}, {member_type_t::TYPE_OBJECT_ARRAY},
          {member_type_t::TYPE_COLOR_ARRAY},  {member_type_t::TYPE_VECTOR3F_ARRAY}, {member_type_t::TYPE_VECTOR3FX_ARRAY},
        };

        static const char *MemberTypeToString(member_type_t mt)
        {
            switch (mt.m_type)
            {
                case member_type_t::TYPE_BOOL8: return "bool";
                case member_type_t::TYPE_INT8: return "s8";
                case member_type_t::TYPE_INT16: return "s16";
                case member_type_t::TYPE_INT32: return "s32";
                case member_type_t::TYPE_INT64: return "int64";
                case member_type_t::TYPE_UINT8: return "u8";
                case member_type_t::TYPE_UINT16: return "u16";
                case member_type_t::TYPE_UINT32: return "u32";
                case member_type_t::TYPE_UINT64: return "uint64";
                case member_type_t::TYPE_FX16: return "fx16";
                case member_type_t::TYPE_FX32: return "fx32";
                case member_type_t::TYPE_FLOAT: return "float";
                case member_type_t::TYPE_STRING: return "string";
                case member_type_t::TYPE_LOCSTR: return "lstring";
                case member_type_t::TYPE_FILEID: return "fileid";
                case member_type_t::TYPE_OBJECT: return "object";
                case member_type_t::TYPE_COLOR: return "color";
                case member_type_t::TYPE_VECTOR3F: return "vector3f";
                case member_type_t::TYPE_VECTOR3FX: return "vector3fx";
                case member_type_t::TYPE_BOOL8_ARRAY: return "bool[]";
                case member_type_t::TYPE_INT8_ARRAY: return "s8[]";
                case member_type_t::TYPE_INT16_ARRAY: return "s16[]";
                case member_type_t::TYPE_INT32_ARRAY: return "s32[]";
                case member_type_t::TYPE_INT64_ARRAY: return "int64[]";
                case member_type_t::TYPE_UINT8_ARRAY: return "u8[]";
                case member_type_t::TYPE_UINT16_ARRAY: return "u16[]";
                case member_type_t::TYPE_UINT32_ARRAY: return "u32[]";
                case member_type_t::TYPE_UINT64_ARRAY: return "uint64[]";
                case member_type_t::TYPE_FX16_ARRAY: return "fx16[]";
                case member_type_t::TYPE_FX32_ARRAY: return "fx32[]";
                case member_type_t::TYPE_FLOAT_ARRAY: return "float[]";
                case member_type_t::TYPE_STRING_ARRAY: return "string[]";
                case member_type_t::TYPE_LOCSTR_ARRAY: return "lstring[]";
                case member_type_t::TYPE_FILEID_ARRAY: return "fileid[]";
                case member_type_t::TYPE_OBJECT_ARRAY: return "object[]";
                case member_type_t::TYPE_COLOR_ARRAY: return "color[]";
                case member_type_t::TYPE_VECTOR3F_ARRAY: return "vector3f[]";
                case member_type_t::TYPE_VECTOR3FX_ARRAY: return "vector3fx[]";
                default: break;
            }
            return "unknown";
        }

        class member_t
        {
            // NOTE: We could optimize this into
            // u8 m_type;
            // u24 m_name_idx; // If the number of members < 16M
            // If all members are in a seperate string table and they are sorted
            // the user logic can get the member idx from the name idx.
            member_type_t m_type;
            u64           m_name_hash;

            struct value_member_t
            {
                u32 m_value;
                u32 m_offset;
            };
            struct array_member_t
            {
                u32 m_array_size;
                u32 m_array_data;
            };

            union
            {
                value_member_t m_value;
                array_member_t m_array;
            };

        public:
            inline member_type_t type() const { return m_type; }
            inline u64           nameHash() const { return m_name_hash; }
            inline u32           offset() const { return m_value.m_offset; }
            inline u32           value() const { return m_value.m_value; }
            inline const u32    *valuePtr() const { return &m_value.m_value; }

            inline bool is_bool() const { return m_type == member_type_t::TYPE_BOOL8; }
            inline bool is_s8() const { return m_type == member_type_t::TYPE_INT8; }
            inline bool is_s16() const { return m_type == member_type_t::TYPE_INT16; }
            inline bool is_s32() const { return m_type == member_type_t::TYPE_INT32; }
            inline bool is_s64() const { return m_type == member_type_t::TYPE_INT64; }
            inline bool is_u8() const { return m_type == member_type_t::TYPE_UINT8; }
            inline bool is_u16() const { return m_type == member_type_t::TYPE_UINT16; }
            inline bool is_u32() const { return m_type == member_type_t::TYPE_UINT32; }
            inline bool is_u64() const { return m_type == member_type_t::TYPE_UINT64; }
            inline bool is_fx16() const { return m_type == member_type_t::TYPE_FX16; }
            inline bool is_fx32() const { return m_type == member_type_t::TYPE_FX32; }
            inline bool is_f32() const { return m_type == member_type_t::TYPE_FLOAT; }
            inline bool is_string() const { return m_type == member_type_t::TYPE_STRING; }
            inline bool is_locstr() const { return m_type == member_type_t::TYPE_LOCSTR; }
            inline bool is_fileid() const { return m_type == member_type_t::TYPE_FILEID; }
            inline bool is_object() const { return m_type == member_type_t::TYPE_OBJECT; }
            inline bool is_color() const { return m_type == member_type_t::TYPE_COLOR; }
            inline bool is_vec3f() const { return m_type == member_type_t::TYPE_VECTOR3F; }
            inline bool is_vec3fx() const { return m_type == member_type_t::TYPE_VECTOR3FX; }
            inline bool is_bool_array() const { return m_type == member_type_t::TYPE_BOOL8_ARRAY; }
            inline bool is_s8_array() const { return m_type == member_type_t::TYPE_INT8_ARRAY; }
            inline bool is_s16_array() const { return m_type == member_type_t::TYPE_INT16_ARRAY; }
            inline bool is_s32_array() const { return m_type == member_type_t::TYPE_INT32_ARRAY; }
            inline bool is_s64_array() const { return m_type == member_type_t::TYPE_INT64_ARRAY; }
            inline bool is_u8_array() const { return m_type == member_type_t::TYPE_UINT8_ARRAY; }
            inline bool is_u16_array() const { return m_type == member_type_t::TYPE_UINT16_ARRAY; }
            inline bool is_u32_array() const { return m_type == member_type_t::TYPE_UINT32_ARRAY; }
            inline bool is_u64_array() const { return m_type == member_type_t::TYPE_UINT64_ARRAY; }
            inline bool is_fx16_array() const { return m_type == member_type_t::TYPE_FX16_ARRAY; }
            inline bool is_fx32_array() const { return m_type == member_type_t::TYPE_FX32_ARRAY; }
            inline bool is_f32_array() const { return m_type == member_type_t::TYPE_FLOAT_ARRAY; }
            inline bool is_string_array() const { return m_type == member_type_t::TYPE_STRING_ARRAY; }
            inline bool is_locstr_array() const { return m_type == member_type_t::TYPE_LOCSTR_ARRAY; }
            inline bool is_fileid_array() const { return m_type == member_type_t::TYPE_FILEID_ARRAY; }
            inline bool is_object_array() const { return m_type == member_type_t::TYPE_OBJECT_ARRAY; }
            inline bool is_color_array() const { return m_type == member_type_t::TYPE_COLOR_ARRAY; }
            inline bool is_vec3f_array() const { return m_type == member_type_t::TYPE_VECTOR3F_ARRAY; }
            inline bool is_vec3fx_array() const { return m_type == member_type_t::TYPE_VECTOR3FX_ARRAY; }
            inline bool isArray() const { return m_type.isArray(); }
            inline bool isPtr() const { return m_type.isPtr(); }

            template <class T>
            const array_t<T> &getArray() const
            {
                const int *data = (const int *)((const char *)&m_offset + m_offset);
                return *((const array_t<T> *)(data));
            }

            u32         sizeOfArray() const { return m_array.m_array_size; }
            const void *dataOfArray() const { return (const void *)((const char *)&m_array.m_array_data + m_array.m_array_data); }
        };

        bool object_t::hasMember(membername_t name) const
        {
            const member_t *m = get_member(name);
            return m != nullptr;
        }

        void object_t::print(const stringtable_t &stringTable, const stringtable_t &typeTable) const
        {
#ifndef _SUBMISSION
            const member_t *member = (const member_t *)((ptr_t)this + sizeof(object_t));
            for (int i = 0; i < getNumMembers(); i++, member++)
            {
                // const char* memberNameStr = stringTable.getStrByHash(member->nameHash());
                // const char* memberTypeStr = typeTable.getStrByIndex(member->typeHash());
                // log_t::writeLine(log_t::INFO, "member_t with name {0} of type {1} with value {2}", va_list_t(va_t(memberNameStr), va_t(memberTypeStr)));
            }
#endif
        }

        bool object_t::get_bool(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getBool(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return false;
            }
#ifndef _SUBMISSION
            if (m->is_bool() == false)
            {
                log_t::writeLine(log_t::WARNING, "getBool(membername_t name): member with name {0} is not of type bool", va_list_t(va_t(name.getName())));
                return false;
            }
#endif
            return m->value() != 0;
        }

        s8 object_t::get_s8(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s8(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s8() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s8(membername_t name): is not an s8.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (s8)m->value();
        }

        u8 object_t::get_u8(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_u8(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u8() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_u8(membername_t name): is not an u8.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (u8)m->value();
        }

        s16 object_t::get_s16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s16() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s16(membername_t name): is not an s16.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (s16)m->value();
        }

        u16 object_t::get_u16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getUs16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u16() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getUs16(membername_t name): is not an int.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (u16)m->value();
        }

        s32 object_t::get_s32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: gets32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s32() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: gets32(membername_t name): is not an int.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return m->value();
        }

        u32 object_t::get_u32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_u32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u32() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_u32(membername_t name): is not an int.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (u32)m->value();
        }

        fx16_t object_t::get_fx16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_fx16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_fx16() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_fx16(membername_t name): is not an fx16.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (fx16_t)m->value();
        }

        fx32_t object_t::get_fx32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfx32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_fx32() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfx32(membername_t name): is not an fx32.", va_list_t(va_t(name.getName())));
                return 0;
            }
#endif
            return (fx32_t)m->value();
        }

        float object_t::get_f32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getFloat(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 1.0f;
            }
#ifndef _SUBMISSION
            if (m->is_f32() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getFloat(membername_t name): is not a float.", va_list_t(va_t(name.getName())));
                return 1.0f;
            }
#endif
            return *((float *)m->valuePtr());
        }

        const char *object_t::get_string(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getString(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return "invalid";
            }
#ifndef _SUBMISSION
            if (m->is_string() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getString(membername_t name): is not a string.", va_list_t(va_t(name.getName())));
                return "invalid";
            }
#endif
            return (const char *)m + m->offset();
        }

        locstr_t object_t::get_locstr(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getlocstr(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return -1;
            }
#ifndef _SUBMISSION
            if (m->is_locstr() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getlocstr(membername_t name): is not a LocId.", va_list_t(va_t(name.getName())));
                return -1;
            }
#endif
            return locstr_t(m->value());
        }

        fileid_t object_t::get_fileid(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfileid(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return INVALID_FILEID;
            }
#ifndef _SUBMISSION
            if (m->is_fileid() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfileid(membername_t name): is not a fileId.", va_list_t(va_t(name.getName())));
                return INVALID_FILEID;
            }
#endif
            return fileid_t(m->value());
        }

        const object_t *object_t::get_object(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getobject(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return nullptr;
            }
#ifndef _SUBMISSION
            if (m->is_object() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getobject(membername_t name): is not a object_t.", va_list_t(va_t(name.getName())));
                return nullptr;
            }

#endif
            if (m->offset() == -1)
                return nullptr;
            else
                return (const object_t *)((const char *)m + m->offset());
        }

        color_t object_t::get_color(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getcolor(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return color_t();
            }
#ifndef _SUBMISSION
            if (m->is_color() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getcolor(membername_t name): is not a xcolor.", va_list_t(va_t(name.getName())));
                return color_t();
            }
#endif
            return color_t((u32)m->value());
        }

        const vec3fx_t &object_t::get_vec3fx(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getVector3Fx(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return vec3fx_t::sZero;
            }
#ifndef _SUBMISSION
            if (m->is_vec3fx() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getVector3Fx(membername_t name): is not a vec3fx_t.", va_list_t(va_t(name.getName())));
                return vec3fx_t::sZero;
            }
#endif
            return *((vec3fx_t *)((const char *)m + m->offset()));
        }

        array_t<bool> object_t::get_bool_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getBoolArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<bool>();
            }
#ifndef _SUBMISSION
            if (m->is_bool_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getBoolArray(membername_t name): is not a vec3fx_t.", va_list_t(va_t(name.getName())));
                return array_t<bool>();
            }
            else
#endif
            {
                const int size = m->sizeOfArray();
                const u8 *data = (const u8 *)m->dataOfArray();
                return array_t<bool>(size, (const bool *)data);
            }
        }

        array_t<s8> object_t::get_s8_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s8Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<s8>();
            }
#ifndef _SUBMISSION
            if (m->is_s8_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s8Array(membername_t name): is not an s8[]", va_list_t(va_t(name.getName())));
                return array_t<s8>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s8>(size, (const s8 *)data);
            }
        }

        array_t<u8> object_t::get_u8_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getu8Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<u8>();
            }
#ifndef _SUBMISSION
            if (m->is_u8_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getu8Array(membername_t name): is not an u8[]", va_list_t(va_t(name.getName())));
                return array_t<u8>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u8>(size, (const u8 *)data);
            }
        }

        array_t<s16> object_t::get_s16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s16Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<s16>();
            }
#ifndef _SUBMISSION
            if (m->is_s16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s16Array(membername_t name): is not an s16[]", va_list_t(va_t(name.getName())));
                return array_t<s16>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s16>(size, (const s16 *)data);
            }
        }

        array_t<u16> object_t::get_u16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getUs16Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<u16>();
            }
#ifndef _SUBMISSION
            if (m->is_u16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getUs16Array(membername_t name): is not an u16[]", va_list_t(va_t(name.getName())));
                return array_t<u16>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u16>(size, (const u16 *)data);
            }
        }

        array_t<s32> object_t::get_s32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: gets32Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<s32>();
            }
#ifndef _SUBMISSION
            if (m->is_s32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: gets32Array(membername_t name): is not an s32[]", va_list_t(va_t(name.getName())));
                return array_t<s32>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s32>(size, (const s32 *)data);
            }
        }

        array_t<u32> object_t::get_u32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getu32Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<u32>();
            }
#ifndef _SUBMISSION
            if (m->is_u32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getu32Array(membername_t name): is not an u32[]", va_list_t(va_t(name.getName())));
                return array_t<u32>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u32>(size, (const u32 *)data);
            }
        }

        array_t<fx16_t> object_t::get_fx16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfx16Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<fx16_t>();
            }
#ifndef _SUBMISSION
            if (m->is_fx16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfx16Array(membername_t name): is not an fx16[]", va_list_t(va_t(name.getName())));
                return array_t<fx16_t>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fx16_t>(size, (const fx16_t *)data);
            }
        }

        array_t<fx32_t> object_t::get_fx32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfx32Array(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<fx32_t>();
            }
#ifndef _SUBMISSION
            if (m->is_fx32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfx32Array(membername_t name): is not an fx32[]", va_list_t(va_t(name.getName())));
                return array_t<fx32_t>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fx32_t>(size, (const fx32_t *)data);
            }
        }

        array_t<f32> object_t::get_f32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getFloatArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<f32>();
            }
#ifndef _SUBMISSION
            if (m->is_f32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getFloatArray(membername_t name): is not an f32[]", va_list_t(va_t(name.getName())));
                return array_t<f32>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<f32>(size, (const f32 *)data);
            }
        }

        array_t<const char *> object_t::get_string_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getStringArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<const char *>();
            }
#ifndef _SUBMISSION
            if (m->is_string_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getStringArray(membername_t name): is not a string[]", va_list_t(va_t(name.getName())));
                return array_t<const char *>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<const char *>(size, (char *const *)data);
            }
        }

        array_t<locstr_t> object_t::get_locstr_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getlocstrArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<locstr_t>();
            }
#ifndef _SUBMISSION
            if (m->is_locstr_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getlocstrArray(membername_t name): is not a locstr_t[]", va_list_t(va_t(name.getName())));
                return array_t<locstr_t>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<locstr_t>(size, (const locstr_t *)data);
            }
        }

        array_t<fileid_t> object_t::get_fileid_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfileidArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<fileid_t>();
            }
#ifndef _SUBMISSION
            if (m->is_fileid_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfileidArray(membername_t name): is not a fileid_t[]", va_list_t(va_t(name.getName())));
                return array_t<fileid_t>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fileid_t>(size, (const fileid_t *)data);
            }
        }

        array_t<const object_t *> object_t::get_object_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getobjectArray(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return array_t<const object_t *>();
            }
#ifndef _SUBMISSION
            if (m->is_object_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getobjectArray(membername_t name): is not a const object_t*[]", va_list_t(va_t(name.getName())));
                return array_t<const object_t *>();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<const object_t *>(size, (object_t *const *)data);
            }
        }

        bool object_t::get_bool(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getBool(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return false;
            }
#ifndef _SUBMISSION
            if (m->is_bool_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getBool(membername_t name, int index): is not a bool[]", va_list_t(va_t(name.getName())));
                return false;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<bool>(size, (const bool *)data)[index] != 0;
            }
        }

        s8 object_t::get_s8(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s8(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s8_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s8(membername_t name, int index): is not a s8[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s8>(size, (const s8 *)data)[index];
            }
        }

        u8 object_t::get_u8(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_u8(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u8_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_u8(membername_t name, int index): is not a u8[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u8>(size, (const u8 *)data)[index];
            }
        }

        s16 object_t::get_s16(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_s16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_s16(membername_t name, int index): is not a s16[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s16>(size, (const s16 *)data)[index];
            }
        }

        u16 object_t::get_u16(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getUs16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getUs16(membername_t name, int index): is not a u16[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u16>(size, (const u16 *)data)[index];
            }
        }

        s32 object_t::get_s32(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: gets32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_s32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: gets32(membername_t name, int index): is not a s32[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<s32>(size, (const s32 *)data)[index];
            }
        }

        u32 object_t::get_u32(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_u32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_u32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_u32(membername_t name, int index): is not a u32[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<u32>(size, (const u32 *)data)[index];
            }
        }

        fx16_t object_t::get_fx16(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: get_fx16(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_fx16_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: get_fx16(membername_t name, int index): is not a fx16[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fx16_t>(size, (const fx16_t *)data)[index];
            }
        }

        fx32_t object_t::get_fx32(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfx32(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return 0;
            }
#ifndef _SUBMISSION
            if (m->is_fx32_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfx32(membername_t name, int index): is not a fx32[]", va_list_t(va_t(name.getName())));
                return 0;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fx32_t>(size, (const fx32_t *)data)[index];
            }
        }

        const char *object_t::get_string(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getString(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return "unknown";
            }
#ifndef _SUBMISSION
            if (m->is_string_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getString(membername_t name, int index): is not a string[]", va_list_t(va_t(name.getName())));
                return "unknown";
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<const char *>(size, (const char **)data)[index];
            }
        }

        locstr_t object_t::get_locstr(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getlocstr(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return INVALID_LOCSTR;
            }
#ifndef _SUBMISSION
            if (m->is_locstr_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getlocstr(membername_t name, int index): is not a locstr_t[]", va_list_t(va_t(name.getName())));
                return locstr_t();
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<locstr_t>(size, (const locstr_t *)data)[index];
            }
        }

        fileid_t object_t::get_fileid(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: getfileid(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return INVALID_FILEID;
            }
#ifndef _SUBMISSION
            if (m->is_fileid_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getfileid(membername_t name, int index): is not a fileid_t[]", va_list_t(va_t(name.getName())));
                return INVALID_FILEID;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<fileid_t>(size, (const fileid_t *)data)[index];
            }
        }

        const object_t *object_t::get_object(membername_t name, int index) const
        {
            const member_t *m = get_member(name);
            if (m == nullptr)
            {
#ifndef _SUBMISSION
                log_t::writeLine(log_t::WARNING, "Warning: geobject(membername_t name): member with name {0} does not exist", va_list_t(va_t(name.getName())));
#endif
                return nullptr;
            }
#ifndef _SUBMISSION
            if (m->is_object_array() == false)
            {
                log_t::writeLine(log_t::WARNING, "Warning: getobject(membername_t name, int index): is not a object_t[]", va_list_t(va_t(name.getName())));
                return nullptr;
            }
            else
#endif
            {
                const int   size = m->sizeOfArray();
                const void *data = m->dataOfArray();
                return array_t<const object_t *>(size, (const object_t **)data)[index];
            }
        }

        // implement FNV1a32 hash function, case insensitive
        static const u64 fnv1a32_basis = 0x811c9dc5;
        static const u64 fnv1a32_prime = 0x1000000001B3ULL;
        static u64       calc_fnv1a32(const char *str)
        {
            u64 hash = fnv1a32_basis;
            while (*str != '\0')
            {
                u32 c = ((u32)*str++);
                if ((c >= 'A') && (c <= 'Z'))
                    c = (c - 'A') + 'a';
                hash = (hash ^ c) * fnv1a32_prime;
            }
            return hash;
        }

        // Note:	We might be able to do a binary search on the member if they
        //			are sorted by name hash!
        const member_t *object_t::get_member(membername_t name) const
        {
            const int nameHash = calc_fnv1a32(name.getName());

            const short *offsets = (const short *)((const char *)this + sizeof(object_t));
            for (int i = 0; i < m_member_count; i++)
            {
                const member_t *member = (member_t *)((const char *)this + sizeof(object_t) + offsets[i]);
                if (member->nameHash() == nameHash)
                    return member;
            }
            return nullptr;
        }

        const void *object_t::get_compound(membername_t name) const
        {
            const int nameHash = calc_fnv1a32(name.getName());

            const short *offsets = (const short *)((const char *)this + sizeof(object_t));
            for (int i = 0; i < m_member_count; i++)
            {
                const member_t *member = (member_t *)((const char *)this + sizeof(object_t) + offsets[i]);
                if (member->nameHash() == nameHash)
                {
                    if (member->offset() != -1)
                        return (const void *)((const char *)member + member->offset());
                }
            }
            return nullptr;
        }

        static bool LogWarningForMemberDoesNotExist(const char *functionName, membername_t name)
        {
#ifndef _SUBMISSION
            log_t::writeLine(log_t::WARNING, "Warning: {0}(membername_t name): member with name {1} does not exist", va_list_t(va_t(functionName), va_t(name.getName())));
#endif
            return false;
        }

        bool object_t::is_s8(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_s8();
            }
            return LogWarningForMemberDoesNotExist("is_s8", name);
        }

        bool object_t::is_s8_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_s8_array();
            }
            return LogWarningForMemberDoesNotExist("is_s8_array", name);
        }

        bool object_t::is_u8(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u8();
            }
            return LogWarningForMemberDoesNotExist("is_u8", name);
        }

        bool object_t::is_u8_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u8_array();
            }
            return LogWarningForMemberDoesNotExist("is_u8_array", name);
        }

        bool object_t::is_s16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u8_array();
            }
            return LogWarningForMemberDoesNotExist("is_s16", name);
        }

        bool object_t::is_s16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u8_array();
            }
            return LogWarningForMemberDoesNotExist("is_s16_array", name);
        }

        bool object_t::is_u16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u16();
            }
            return LogWarningForMemberDoesNotExist("is_u16", name);
        }

        bool object_t::is_u16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u16_array();
            }
            return LogWarningForMemberDoesNotExist("is_u16_array", name);
        }

        bool object_t::is_s32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_s32();
            }
            return LogWarningForMemberDoesNotExist("is_s32", name);
        }

        bool object_t::is_s32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_s32_array();
            }
            return LogWarningForMemberDoesNotExist("is_s32_array", name);
        }

        bool object_t::is_u32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u32();
            }
            return LogWarningForMemberDoesNotExist("is_u32", name);
        }

        bool object_t::is_u32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_u32_array();
            }
            return LogWarningForMemberDoesNotExist("is_u32_array", name);
        }

        bool object_t::is_fx16(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fx16();
            }
            return LogWarningForMemberDoesNotExist("is_fx16", name);
        }

        bool object_t::is_fx16_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fx16_array();
            }
            return LogWarningForMemberDoesNotExist("is_fx16_array", name);
        }

        bool object_t::is_fx32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fx32();
            }
            return LogWarningForMemberDoesNotExist("is_fx32", name);
        }

        bool object_t::is_fx32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fx32_array();
            }
            return LogWarningForMemberDoesNotExist("is_fx32_array", name);
        }

        bool object_t::is_f32(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_f32();
            }
            return LogWarningForMemberDoesNotExist("is_f32", name);
        }

        bool object_t::is_f32_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_f32_array();
            }
            return LogWarningForMemberDoesNotExist("is_f32_array", name);
        }

        bool object_t::is_locstr(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_locstr();
            }
            return LogWarningForMemberDoesNotExist("is_locstr", name);
        }

        bool object_t::is_locstr_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_locstr_array();
            }
            return LogWarningForMemberDoesNotExist("is_locstr_array", name);
        }

        bool object_t::is_fileid(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fileid();
            }
            return LogWarningForMemberDoesNotExist("is_fileid", name);
        }

        bool object_t::is_fileid_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_fileid_array();
            }
            return LogWarningForMemberDoesNotExist("is_fileid_array", name);
        }

        bool object_t::is_color(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_color();
            }
            return LogWarningForMemberDoesNotExist("is_color", name);
        }

        bool object_t::is_color_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_color_array();
            }
            return LogWarningForMemberDoesNotExist("is_color_array", name);
        }

        bool object_t::is_object(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_object();
            }
            return LogWarningForMemberDoesNotExist("is_object", name);
        }

        bool object_t::is_object_array(membername_t name) const
        {
            const member_t *m = get_member(name);
            if (m != nullptr)
            {
                return m->is_object_array();
            }
            return LogWarningForMemberDoesNotExist("is_object_array", name);
        }
    }  // namespace ngd
}  // namespace ncore