#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "cfile/c_file.h"

#include "cunittest/cunittest.h"
#include "charon/test_allocator.h"
#include "charon/c_object.h"
#include "charon/c_root.h"

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
            const char*          filename = "metatest.cdd";
            nfile::file_handle_t fh       = nfile::file_open(filename, nfile::FILE_MODE_READ);
            s32                  dataSize = (s32)nfile::file_size(fh);
            u8*                  data     = (u8*)Allocator->allocate(dataSize);
            nfile::file_read(fh, data, dataSize);

            ngd::root_t* root = (ngd::root_t*)data;

            Allocator->deallocate(data);
        }
    }
}
UNITTEST_SUITE_END
