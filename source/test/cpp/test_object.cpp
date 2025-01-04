#include "ccore/c_target.h"

#include "cunittest/cunittest.h"
#include "ccore/c_allocator.h"
#include "cfile/c_file.h"
#include "charon/c_gamedata.h"
#include "charon/c_archive.h"

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

            charon::dataunit_header_t* dataUnit = (charon::dataunit_header_t*)data;
            charon::gameroot_t* root = (charon::gameroot_t*)charon::g_patch(dataUnit);

            charon::tracks_t const*                    tracks = root->getTracks();
            charon::dataunit_t<charon::track_t> const& track  = tracks->getTracks()[0];

            Allocator->deallocate(data);
        }
    }
}
UNITTEST_SUITE_END
