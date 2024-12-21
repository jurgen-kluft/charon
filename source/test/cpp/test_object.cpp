#include "ccore/c_target.h"

#include "cunittest/cunittest.h"
#include "charon/c_object.h"
#include "ccore/c_allocator.h"
#include "cfile/c_file.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(gamedata)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(create)
        {
            const char*          filename = "GameData.bfd";
            nfile::file_handle_t fh       = nfile::file_open(filename, nfile::FILE_MODE_READ);
            s32                  dataSize = (s32)nfile::file_size(fh);
            u8*                  data     = (u8*)Allocator->allocate(dataSize);
            nfile::file_read(fh, data, dataSize);

            charon::gameroot_t* root = (charon::gameroot_t*)data;

            s32* pointer = (s32*)&root->m_Tracks;
            while (true)
            {
                s32 const nextOffset = pointer[0];
                s32 const dataOffset = pointer[1];

                void** pointerToPatch = (void**)pointer;
                *pointerToPatch = (void*)((uptr_t)pointer + dataOffset);

                if (nextOffset == 0)
                    break;
                pointer = (s32*)((uptr_t)pointer + nextOffset);
            }

            charon::tracks_t const* tracks = root->m_Tracks;

            Allocator->deallocate(data);
        }
    }
}
UNITTEST_SUITE_END
